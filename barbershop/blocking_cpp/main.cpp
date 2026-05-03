// Blocking barbershop implementation:
// Customers enter a bounded waiting room under a mutex. The barber sleeps on a
// condition variable until a customer arrives or the shop closes.
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
  constexpr int chairs = 2;
  std::mutex mutex;
  std::condition_variable customer_ready;
  std::condition_variable haircut_done;
  int waiting = 0;
  int served = 0;
  bool open = true;

  std::thread barber([&] {
    for (;;) {
      std::unique_lock<std::mutex> lock(mutex);
      customer_ready.wait(lock, [&] { return waiting > 0 || !open; });
      if (waiting == 0 && !open) break;
      --waiting;
      ++served;
      std::cout << "barber served customer " << served << '\n';
      haircut_done.notify_one();
    }
  });

  std::vector<std::thread> customers;
  for (int id = 1; id <= 5; ++id) {
    customers.emplace_back([&, id] {
      std::unique_lock<std::mutex> lock(mutex);
      if (waiting == chairs) {
        std::cout << "customer " << id << " left\n";
        return;
      }
      ++waiting;
      std::cout << "customer " << id << " waited\n";
      customer_ready.notify_one();
      haircut_done.wait(lock);
    });
  }

  for (auto& customer : customers) customer.join();
  {
    std::lock_guard<std::mutex> lock(mutex);
    open = false;
  }
  customer_ready.notify_one();
  barber.join();
}
