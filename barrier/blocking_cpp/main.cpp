// Blocking barrier implementation:
// std::barrier blocks each worker at arrive_and_wait until all three workers
// have reached the rendezvous point, then releases the whole group together.
#include <barrier>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

std::mutex print_mutex;

void log(const std::string& message) {
  std::lock_guard<std::mutex> lock(print_mutex);
  std::cout << message << '\n';
}

int main() {
  std::barrier sync_point(3, [] { log("all workers reached the barrier"); });
  std::vector<std::thread> workers;

  for (int id = 0; id < 3; ++id) {
    workers.emplace_back([id, &sync_point] {
      log("worker " + std::to_string(id) + " before barrier");
      sync_point.arrive_and_wait();
      log("worker " + std::to_string(id) + " after barrier");
    });
  }

  for (auto& worker : workers) worker.join();
}
