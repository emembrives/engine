// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/ios/FlutterDartSource.h"

@implementation FlutterDartSource {
  NSURL* _dartMain;
  NSURL* _packages;
  NSURL* _flxArchive;
}

- (instancetype)init {
  return [self initWithDartMain:nil packages:nil flxArchive:nil];
}

- (instancetype)initWithDartMain:(NSURL*)dartMain
                        packages:(NSURL*)packages
                      flxArchive:(NSURL*)flxArchive {
  self = [super init];

  if (self) {
    _dartMain = [dartMain copy];
    _packages = [packages copy];
    _flxArchive = [flxArchive copy];
  }

  return self;
}

static BOOL CheckDartProjectURL(NSMutableString* log,
                                NSURL* url,
                                NSString* logLabel) {
  if (url == nil) {
    [log appendFormat:@"The %@ was not specified.\n", logLabel];
    return false;
  }

  if (!url.isFileURL) {
    [log appendFormat:@"The %@ must be a file URL.\n", logLabel];
    return false;
  }

  if (![[NSFileManager defaultManager] fileExistsAtPath:url.absoluteURL.path]) {
    [log appendFormat:@"No file found at '%@' when looking for the %@.\n", url,
                      logLabel];
    return false;
  }

  return true;
}

- (void)validate:(ValidationResult)result {
  NSMutableString* log = [[[NSMutableString alloc] init] autorelease];

  BOOL isValid = YES;

  isValid &= CheckDartProjectURL(log, _flxArchive, @"FLX archive");
  isValid &= CheckDartProjectURL(log, _dartMain, @"Dart main");
  isValid &= CheckDartProjectURL(log, _packages, @"Dart packages");

  result(isValid, log);
}

- (void)dealloc {
  [_dartMain release];
  [_packages release];
  [_flxArchive release];

  [super dealloc];
}

@end
