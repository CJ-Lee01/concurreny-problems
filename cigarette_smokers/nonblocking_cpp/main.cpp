// Nonblocking cigarette smokers implementation:
// An atomic table slot names the smoker that can proceed. Smokers CAS the table
// back to empty when their needed resources appear, then count completion.
#include <atomic>
#include <iostream>
#include <string>
#include <thread>

int main() {
  std::atomic<int> table{0};
  std::atomic<int> completed{0};

  auto smoker = [&](int need, const std::string& name) {
    while (completed.load(std::memory_order_acquire) < 6) {
      int expected = need;
      if (table.compare_exchange_strong(expected, 0, std::memory_order_acq_rel)) {
        std::cout << name << " smoker made a cigarette\n";
        completed.fetch_add(1, std::memory_order_acq_rel);
      } else {
        std::this_thread::yield();
      }
    }
  };

  std::thread tobacco(smoker, 1, "tobacco");
  std::thread paper(smoker, 2, "paper");
  std::thread matches(smoker, 3, "matches");

  for (int round = 0; round < 6; ++round) {
    while (table.load(std::memory_order_acquire) != 0) std::this_thread::yield();
    table.store((round % 3) + 1, std::memory_order_release);
  }

  tobacco.join();
  paper.join();
  matches.join();
}
