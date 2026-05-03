// Blocking FIFO semaphore implementation:
// Waiters enqueue their private condition variable. A release wakes only the
// front waiter, so permits are granted in arrival order.
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class FifoSemaphore {
 public:
  explicit FifoSemaphore(int permits) : permits_(permits) {}

  void acquire() {
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(mutex_);
    waiters_.push(&cv);
    cv.wait(lock, [&] { return waiters_.front() == &cv && permits_ > 0; });
    --permits_;
    waiters_.pop();
  }

  void release() {
    std::lock_guard<std::mutex> lock(mutex_);
    ++permits_;
    if (!waiters_.empty()) waiters_.front()->notify_one();
  }

 private:
  int permits_;
  std::queue<std::condition_variable*> waiters_;
  std::mutex mutex_;
};

int main() {
  FifoSemaphore semaphore(1);
  std::vector<std::thread> workers;

  for (int id = 0; id < 4; ++id) {
    workers.emplace_back([&, id] {
      semaphore.acquire();
      std::cout << "fifo worker " << id << '\n';
      semaphore.release();
    });
  }

  for (auto& worker : workers) worker.join();
}
