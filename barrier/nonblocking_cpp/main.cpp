// Nonblocking barrier implementation:
// Workers decrement an atomic arrival counter and spin on an atomic generation
// number. The last worker resets the counter and advances the generation.
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class SpinBarrier {
 public:
  explicit SpinBarrier(int parties) : parties_(parties), remaining_(parties) {}

  void arrive_and_wait() {
    int generation = generation_.load(std::memory_order_acquire);
    if (remaining_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      remaining_.store(parties_, std::memory_order_release);
      generation_.fetch_add(1, std::memory_order_release);
      return;
    }
    while (generation_.load(std::memory_order_acquire) == generation) std::this_thread::yield();
  }

 private:
  int parties_;
  std::atomic<int> remaining_;
  std::atomic<int> generation_{0};
};

std::atomic_flag print_lock = ATOMIC_FLAG_INIT;

void log(const std::string& message) {
  while (print_lock.test_and_set(std::memory_order_acquire)) std::this_thread::yield();
  std::cout << message << '\n';
  print_lock.clear(std::memory_order_release);
}

int main() {
  SpinBarrier barrier(3);
  std::vector<std::thread> workers;

  for (int id = 0; id < 3; ++id) {
    workers.emplace_back([id, &barrier] {
      log("worker " + std::to_string(id) + " before barrier");
      barrier.arrive_and_wait();
      log("worker " + std::to_string(id) + " after barrier");
    });
  }

  for (auto& worker : workers) worker.join();
}
