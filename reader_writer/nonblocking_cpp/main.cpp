// Nonblocking reader-writer implementation:
// A single atomic integer stores the lock state: nonnegative values count
// readers, and -1 marks a writer. Threads acquire by CAS loops and yield.
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class SpinRwLock {
 public:
  void read_lock() {
    for (;;) {
      int state = state_.load(std::memory_order_acquire);
      if (state >= 0 && state_.compare_exchange_weak(state, state + 1, std::memory_order_acq_rel)) return;
      std::this_thread::yield();
    }
  }

  void read_unlock() { state_.fetch_sub(1, std::memory_order_release); }

  void write_lock() {
    int expected = 0;
    while (!state_.compare_exchange_weak(expected, -1, std::memory_order_acq_rel)) {
      expected = 0;
      std::this_thread::yield();
    }
  }

  void write_unlock() { state_.store(0, std::memory_order_release); }

 private:
  std::atomic<int> state_{0};
};

int main() {
  SpinRwLock lock;
  int shared_value = 0;

  std::thread writer([&] {
    lock.write_lock();
    shared_value = 42;
    std::cout << "writer stored 42\n";
    lock.write_unlock();
  });
  writer.join();

  std::vector<std::thread> readers;
  for (int id = 0; id < 3; ++id) {
    readers.emplace_back([&, id] {
      lock.read_lock();
      std::cout << "reader " << id << " saw " << shared_value << '\n';
      lock.read_unlock();
    });
  }
  for (auto& reader : readers) reader.join();
}
