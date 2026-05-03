// Nonblocking producer-consumer implementation:
// A small ring buffer uses atomic head/tail counters and per-slot readiness
// flags. Producer and consumer retry with compare-and-swap and yield on failure.
#include <atomic>
#include <iostream>
#include <thread>

class MpmcRing {
 public:
  bool push(int value) {
    for (;;) {
      std::size_t tail = tail_.load(std::memory_order_acquire);
      std::size_t head = head_.load(std::memory_order_acquire);
      if (tail - head == capacity_) return false;
      if (tail_.compare_exchange_weak(tail, tail + 1, std::memory_order_acq_rel)) {
        buffer_[tail % capacity_].store(value, std::memory_order_release);
        ready_[tail % capacity_].store(true, std::memory_order_release);
        return true;
      }
    }
  }

  bool pop(int& value) {
    for (;;) {
      std::size_t head = head_.load(std::memory_order_acquire);
      std::size_t tail = tail_.load(std::memory_order_acquire);
      if (head == tail) return false;
      bool expected = true;
      if (!ready_[head % capacity_].compare_exchange_weak(expected, false, std::memory_order_acq_rel)) {
        std::this_thread::yield();
        continue;
      }
      value = buffer_[head % capacity_].load(std::memory_order_acquire);
      while (!head_.compare_exchange_weak(head, head + 1, std::memory_order_acq_rel)) std::this_thread::yield();
      return true;
    }
  }

 private:
  static constexpr std::size_t capacity_ = 4;
  std::atomic<int> buffer_[capacity_]{};
  std::atomic<bool> ready_[capacity_]{};
  std::atomic<std::size_t> head_{0};
  std::atomic<std::size_t> tail_{0};
};

int main() {
  MpmcRing queue;
  std::atomic<bool> done{false};
  std::thread producer([&] {
    for (int value = 1; value <= 5; ++value) {
      while (!queue.push(value)) std::this_thread::yield();
    }
    done.store(true, std::memory_order_release);
  });
  std::thread consumer([&] {
    int value = 0;
    while (!done.load(std::memory_order_acquire)) {
      if (queue.pop(value)) std::cout << "consumed " << value << '\n';
      else std::this_thread::yield();
    }
    while (queue.pop(value)) std::cout << "consumed " << value << '\n';
  });
  producer.join();
  consumer.join();
}
