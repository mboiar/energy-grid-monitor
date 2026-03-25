#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>

class PeriodicTask {
public:
  PeriodicTask(std::chrono::milliseconds interval)
      : interval_(interval), running_(true) {}

  virtual ~PeriodicTask() { stop(); }

  void start() {
    worker_ = std::thread([this] { run(); });
  }

  void stop() {
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable())
      worker_.join();
  }

protected:
  virtual void callback() = 0;

private:
  void run() {
    auto next = std::chrono::steady_clock::now() + interval_;
    while (running_) {
      // Wait until next execution time or stop signal
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait_until(lock, next, [this] { return !running_; });
      if (!running_)
        break;

      // Perform the task
      callback();

      // Schedule next iteration
      next += interval_;
    }
  }

  std::chrono::milliseconds interval_;
  std::atomic<bool> running_;
  std::thread worker_;
  std::mutex mutex_;
  std::condition_variable cv_;
};