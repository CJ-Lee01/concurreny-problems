// Blocking reader-writer implementation:
// std::shared_mutex allows many concurrent shared reader locks while the writer
// takes an exclusive lock before updating the shared value.
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

int main() {
  std::shared_mutex mutex;
  int shared_value = 0;

  std::thread writer([&] {
    std::unique_lock lock(mutex);
    shared_value = 42;
    std::cout << "writer stored 42\n";
  });
  writer.join();

  std::vector<std::thread> readers;
  for (int id = 0; id < 3; ++id) {
    readers.emplace_back([&, id] {
      std::shared_lock lock(mutex);
      std::cout << "reader " << id << " saw " << shared_value << '\n';
    });
  }
  for (auto& reader : readers) reader.join();
}
