#ifndef SEXP7_THREAD_SAFE_QUEUE_HPP
#define SEXP7_THREAD_SAFE_QUEUE_HPP

#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class ThreadSafeQueue {
private:
  std::queue<T> queue;
  std::mutex mutex;

public:
  void push(T&& item) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.emplace(std::move(item));
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
      return std::nullopt;
    }
    T item = std::move(queue.front());
    queue.pop();
    return item;
  }
};
#endif
