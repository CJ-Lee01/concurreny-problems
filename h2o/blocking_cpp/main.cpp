// Blocking H2O implementation:
// Counting semaphores admit at most two hydrogens and one oxygen per molecule.
// A barrier waits for all three atoms, then releases permits for the next group.
#include <barrier>
#include <iostream>
#include <mutex>
#include <semaphore>
#include <thread>
#include <vector>

std::mutex print_mutex;

void print(const char* text) {
  std::lock_guard<std::mutex> lock(print_mutex);
  std::cout << text << '\n';
}

int main() {
  std::counting_semaphore<2> hydrogen(2);
  std::counting_semaphore<1> oxygen(1);
  std::barrier molecule(3, [&] {
    print("formed H2O");
    hydrogen.release(2);
    oxygen.release();
  });

  auto h = [&] {
    hydrogen.acquire();
    print("H ready");
    molecule.arrive_and_wait();
  };
  auto o = [&] {
    oxygen.acquire();
    print("O ready");
    molecule.arrive_and_wait();
  };

  std::vector<std::thread> atoms;
  for (int i = 0; i < 4; ++i) atoms.emplace_back(h);
  for (int i = 0; i < 2; ++i) atoms.emplace_back(o);
  for (auto& atom : atoms) atom.join();
}
