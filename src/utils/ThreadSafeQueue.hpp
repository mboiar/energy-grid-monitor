#pragma once

#include <optional>
#include <queue>

#include <condition_variable>
#include <mutex>

template <typename T> class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  void push(T &item) {
    std::lock_guard<std::mutex> lock(mtx_);
    queue_.push(item);
    cv_.notify_one();
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (queue_.empty())
      return std::nullopt;
    T item = queue_.front();
    queue_.pop();
    return item;
  }

  T wait_and_pop() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    T item = queue_.front();
    queue_.pop();
    return item;
  }

  template <typename Rep, typename Period>
  bool wait_and_pop_for(T &val,
                        const std::chrono::duration<Rep, Period> &timeout) {
    std::unique_lock<std::mutex> lock(mtx_);
    bool ok = cv_.wait_for(lock, timeout, [&] { return !queue_.empty(); });

    if (ok) {
      val = queue_.front();
      queue_.pop();
    }

    return ok;
  }

private:
  std::queue<T> queue_;
  std::mutex mtx_;
  std::condition_variable cv_;
};
