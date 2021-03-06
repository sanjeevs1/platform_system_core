/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LIBBACKTRACE_BACKTRACE_THREAD_H
#define _LIBBACKTRACE_BACKTRACE_THREAD_H

#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <ucontext.h>

#include "BacktraceImpl.h"

// The signal used to cause a thread to dump the stack.
#if defined(__GLIBC__)
// GLIBC reserves __SIGRTMIN signals, so use SIGRTMIN to avoid errors.
#define THREAD_SIGNAL SIGRTMIN
#else
#define THREAD_SIGNAL (__SIGRTMIN+1)
#endif

class ThreadEntry {
public:
  static ThreadEntry* Get(pid_t pid, pid_t tid, bool create = true);

  static void Remove(ThreadEntry* entry);

  inline void CopyUcontext(ucontext_t* ucontext) {
    memcpy(&ucontext_, ucontext, sizeof(ucontext_));
  }

  void Wake();

  void Wait(int);

  inline void Lock() {
    pthread_mutex_lock(&mutex_);
    // Reset the futex value in case of multiple unwinds of the same thread.
    futex_ = 0;
  }

  inline void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

  inline ucontext_t* GetUcontext() { return &ucontext_; }

private:
  ThreadEntry(pid_t pid, pid_t tid);
  ~ThreadEntry();

  bool Match(pid_t chk_pid, pid_t chk_tid) { return (chk_pid == pid_ && chk_tid == tid_); }

  pid_t pid_;
  pid_t tid_;
  int futex_;
  int ref_count_;
  pthread_mutex_t mutex_;
  ThreadEntry* next_;
  ThreadEntry* prev_;
  ucontext_t ucontext_;

  static ThreadEntry* list_;
  static pthread_mutex_t list_mutex_;
};

class BacktraceThread : public BacktraceCurrent {
public:
  BacktraceThread(BacktraceImpl* impl, pid_t tid, BacktraceMap* map);
  virtual ~BacktraceThread();

  virtual bool Unwind(size_t num_ignore_frames, ucontext_t* ucontext);
};

#endif // _LIBBACKTRACE_BACKTRACE_THREAD_H
