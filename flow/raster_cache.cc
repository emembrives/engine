// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow/raster_cache.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkSurface.h"

#define ENABLE_RASTER_CACHE 1

namespace flow {

#if ENABLE_RASTER_CACHE

static const int kRasterThreshold = 3;

static bool isWorthRasterizing(SkPicture* picture) {
  // TODO(abarth): We should find a better heuristic here that lets us avoid
  // wasting memory on trivial layers that are easy to re-rasterize every frame.
  return picture->approximateOpCount() > 10 || picture->hasText();
}

#endif

RasterCache::RasterCache() {
}

RasterCache::~RasterCache() {
}

RasterCache::Entry::Entry() {
  physical_size.setEmpty();
}

RasterCache::Entry::~Entry() {
}

skia::RefPtr<SkImage> RasterCache::GetPrerolledImage(GrContext* context,
                                                     SkPicture* picture,
                                                     const SkMatrix& ctm) {
#if ENABLE_RASTER_CACHE
  SkScalar scaleX = ctm.getScaleX();
  SkScalar scaleY = ctm.getScaleY();

  SkRect rect = picture->cullRect();

  SkISize physical_size = SkISize::Make(rect.width() * scaleX,
                                        rect.height() * scaleY);

  if (physical_size.isEmpty())
    return nullptr;

  Entry& entry = cache_[picture->uniqueID()];

  const bool size_matched = entry.physical_size == physical_size;

  entry.used_this_frame = true;
  entry.physical_size = physical_size;

  if (!size_matched) {
    entry.access_count = 1;
    entry.image = nullptr;
    return nullptr;
  }

  entry.access_count++;

  if (entry.access_count >= kRasterThreshold) {
    // Saturate at the threshhold.
    entry.access_count = kRasterThreshold;

    if (!entry.image && isWorthRasterizing(picture)) {
      TRACE_EVENT2("flutter", "Rasterize picture layer",
                   "width", physical_size.width(),
                   "height", physical_size.height());
      SkImageInfo info = SkImageInfo::MakeN32Premul(physical_size);
      skia::RefPtr<SkSurface> surface = skia::AdoptRef(
          SkSurface::NewRenderTarget(context, SkBudgeted::kYes, info));
      if (surface) {
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->scale(scaleX, scaleY);
        canvas->translate(-rect.left(), -rect.top());
        canvas->drawPicture(picture);
        entry.image = skia::AdoptRef(surface->newImageSnapshot());
      }
    }
  }

  return entry.image;
#else
  return nullptr;
#endif
}

void RasterCache::SweepAfterFrame() {
  std::vector<Cache::iterator> dead;

  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    Entry& entry = it->second;
    if (!entry.used_this_frame)
      dead.push_back(it);
    entry.used_this_frame = false;
  }

  for (auto it : dead)
    cache_.erase(it);
}

void RasterCache::Clear() {
  cache_.clear();
}

}  // namespace flow
