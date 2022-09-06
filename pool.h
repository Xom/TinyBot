#ifndef TINYBOT_POOL_H
#define TINYBOT_POOL_H

#include <condition_variable>
#include <mutex>
#include <stack>

namespace tinybot {

template <class T>
class Pool {
 public:
  Pool() = default;
  Pool(const Pool&) = delete;
  Pool& operator=(const Pool&) = delete;
  Pool(Pool&&) = delete;
  Pool& operator=(Pool&&) = delete;
  // TODO implement destructor if I ever want to use Pool for objects that should be garbage-collected before program termination

  void push(T* ptr) {
    std::unique_lock<std::mutex> lock(mutex);
    stack.push(ptr);
    condition_variable.notify_one();
  }

  T* pop() {
    std::unique_lock<std::mutex> lock(mutex);
    condition_variable.wait(lock, [this] { return !stack.empty(); });
    T* ptr = stack.top();
    stack.pop();
    return ptr;
  }

 private:
  std::stack<T*> stack{};
  std::mutex mutex{};
  std::condition_variable condition_variable{};
};

}  // namespace tinybot

#endif  // TINYBOT_POOL_H
