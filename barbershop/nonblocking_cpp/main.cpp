// Nonblocking barbershop implementation:
// Atomic counters track waiting customers and completed haircuts. Customers use
// CAS to claim a chair, while the barber spins/yields until work is available.
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
  constexpr int chairs = 2;
  std::atomic<int> waiting{0};
  std::atomic<int> served{0};
  std::atomic<bool> open{true};

  std::thread barber([&] {
    while (open.load(std::memory_order_acquire) || waiting.load(std::memory_order_acquire) > 0) {
      int current = waiting.load(std::memory_order_acquire);
      if (current > 0 && waiting.compare_exchange_weak(current, current - 1, std::memory_order_acq_rel)) {
        int id = served.fetch_add(1, std::memory_order_acq_rel) + 1;
        std::cout << "barber served customer " << id << '\n';
      } else {
        std::this_thread::yield();
      }
    }
  });

  std::vector<std::thread> customers;
  for (int id = 1; id <= 5; ++id) {
    customers.emplace_back([&, id] {
      for (;;) {
        int current = waiting.load(std::memory_order_acquire);
        if (current == chairs) {
          std::cout << "customer " << id << " left\n";
          return;
        }
        if (waiting.compare_exchange_weak(current, current + 1, std::memory_order_acq_rel)) {
          std::cout << "customer " << id << " waited\n";
          return;
        }
      }
    });
  }

  for (auto& customer : customers) customer.join();
  open.store(false, std::memory_order_release);
  barber.join();
}
