// Blocking producer-consumer implementation:
// A bounded FIFO queue is protected by a mutex. Condition variables block the
// producer when the buffer is full and the consumer when the buffer is empty.
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class BoundedBuffer {
 public:
  explicit BoundedBuffer(std::size_t capacity) : capacity_(capacity) {}

  void push(int value) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [&] { return queue_.size() < capacity_; });
    queue_.push(value);
    not_empty_.notify_one();
  }

  int pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [&] { return !queue_.empty(); });
    int value = queue_.front();
    queue_.pop();
    not_full_.notify_one();
    return value;
  }

 private:
  std::size_t capacity_;
  std::queue<int> queue_;
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
};

int main() {
  BoundedBuffer buffer(2);
  std::thread producer([&] {
    for (int value = 1; value <= 5; ++value) buffer.push(value);
    buffer.push(-1);
  });
  std::thread consumer([&] {
    for (;;) {
      int value = buffer.pop();
      if (value == -1) break;
      std::cout << "consumed " << value << '\n';
    }
  });
  producer.join();
  consumer.join();
}
