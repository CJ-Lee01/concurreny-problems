// Blocking dining philosophers implementation:
// Each fork is a mutex. Philosophers always lock the lower-numbered fork first,
// then the higher-numbered fork, which removes circular wait and deadlock.
#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
  std::vector<std::mutex> forks(5);
  std::vector<std::thread> philosophers;

  for (int id = 0; id < 5; ++id) {
    philosophers.emplace_back([&, id] {
      int left = id;
      int right = (id + 1) % 5;
      std::scoped_lock lock(forks[std::min(left, right)], forks[std::max(left, right)]);
      std::cout << "philosopher " << id << " ate\n";
    });
  }

  for (auto& philosopher : philosophers) philosopher.join();
}
