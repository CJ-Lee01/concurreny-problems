// Nonblocking FIFO semaphore implementation:
// Each acquirer takes an atomic ticket and spins until its ticket is serving and
// a permit is available, giving first-in-first-out admission.
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class FifoSpinSemaphore {
 public:
  explicit FifoSpinSemaphore(int permits) : permits_(permits) {}

  void acquire() {
    int ticket = next_.fetch_add(1, std::memory_order_acq_rel);
    for (;;) {
      int current = permits_.load(std::memory_order_acquire);
      if (serving_.load(std::memory_order_acquire) == ticket && current > 0 &&
          permits_.compare_exchange_weak(current, current - 1, std::memory_order_acq_rel)) {
        serving_.fetch_add(1, std::memory_order_release);
        return;
      }
      std::this_thread::yield();
    }
  }

  void release() { permits_.fetch_add(1, std::memory_order_release); }

 private:
  std::atomic<int> permits_;
  std::atomic<int> next_{0};
  std::atomic<int> serving_{0};
};

int main() {
  FifoSpinSemaphore semaphore(1);
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
