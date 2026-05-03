// Nonblocking H2O implementation:
// Atomic spin semaphores limit the atom mix to two hydrogens and one oxygen.
// A spin barrier groups exactly three admitted atoms before permits are returned.
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class SpinSemaphore {
 public:
  explicit SpinSemaphore(int permits) : permits_(permits) {}

  void acquire() {
    for (;;) {
      int current = permits_.load(std::memory_order_acquire);
      if (current > 0 &&
          permits_.compare_exchange_weak(current, current - 1, std::memory_order_acq_rel)) {
        return;
      }
      std::this_thread::yield();
    }
  }

  void release() { permits_.fetch_add(1, std::memory_order_release); }

 private:
  std::atomic<int> permits_;
};

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

int main() {
  SpinSemaphore hydrogen(2);
  SpinSemaphore oxygen(1);
  SpinBarrier molecule(3);

  auto h = [&] {
    hydrogen.acquire();
    std::cout << "H ready\n";
    molecule.arrive_and_wait();
    hydrogen.release();
  };
  auto o = [&] {
    oxygen.acquire();
    std::cout << "O ready\n";
    molecule.arrive_and_wait();
    oxygen.release();
  };

  std::vector<std::thread> atoms;
  for (int i = 0; i < 4; ++i) atoms.emplace_back(h);
  for (int i = 0; i < 2; ++i) atoms.emplace_back(o);
  for (auto& atom : atoms) atom.join();
}
