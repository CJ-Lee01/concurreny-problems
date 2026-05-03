// Nonblocking dining philosophers implementation:
// Each fork is an atomic_flag spin lock. Philosophers acquire forks in numeric
// order, so the CAS-style spinning cannot form a circular wait.
#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
  std::atomic_flag forks[5];
  std::vector<std::thread> philosophers;

  for (int id = 0; id < 5; ++id) {
    philosophers.emplace_back([&, id] {
      int left = id;
      int right = (id + 1) % 5;
      int first = std::min(left, right);
      int second = std::max(left, right);
      while (forks[first].test_and_set(std::memory_order_acquire)) std::this_thread::yield();
      while (forks[second].test_and_set(std::memory_order_acquire)) std::this_thread::yield();
      std::cout << "philosopher " << id << " ate\n";
      forks[second].clear(std::memory_order_release);
      forks[first].clear(std::memory_order_release);
    });
  }

  for (auto& philosopher : philosophers) philosopher.join();
}
