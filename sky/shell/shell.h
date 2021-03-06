// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_SHELL_H_
#define SKY_SHELL_SHELL_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "sky/shell/tracing_controller.h"

namespace sky {
namespace shell {

class Rasterizer;

class Shell {
 public:
  ~Shell();

  // Init the shell to stand alone from MojoShell.
  static void InitStandalone(std::string icu_data_path = "");

  // Init the shell to run inside MojoShell.
  static void Init();

  static Shell& Shared();

  base::SingleThreadTaskRunner* gpu_task_runner() const {
    return gpu_task_runner_.get();
  }

  base::SingleThreadTaskRunner* ui_task_runner() const {
    return ui_task_runner_.get();
  }

  base::SingleThreadTaskRunner* io_task_runner() const {
    return io_task_runner_.get();
  }

  TracingController& tracing_controller();

  // Maintain a list of rasterizers.
  // These APIs must only be accessed on the GPU thread.
  void AddRasterizer(const base::WeakPtr<Rasterizer>& rasterizer);
  void PurgeRasterizers();
  void GetRasterizers(std::vector<base::WeakPtr<Rasterizer>>* rasterizer);

 private:
  Shell();

  void InitGpuThread();

  std::unique_ptr<base::Thread> gpu_thread_;
  std::unique_ptr<base::Thread> ui_thread_;
  std::unique_ptr<base::Thread> io_thread_;

  scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;

  std::unique_ptr<base::ThreadChecker> gpu_thread_checker_;

  TracingController tracing_controller_;

  std::vector<base::WeakPtr<Rasterizer>> rasterizers_;

  DISALLOW_COPY_AND_ASSIGN(Shell);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_SHELL_H_
