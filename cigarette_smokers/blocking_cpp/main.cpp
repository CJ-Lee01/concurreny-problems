// Blocking cigarette smokers implementation:
// The agent semaphore ensures one table placement at a time. Each smoker waits
// on the semaphore representing the missing ingredient pair it needs.
#include <iostream>
#include <semaphore>
#include <string>
#include <thread>

int main() {
  std::binary_semaphore agent(1), tobacco(0), paper(0), matches(0);

  auto smoker = [&](const std::string& name, std::binary_semaphore& need) {
    for (int i = 0; i < 2; ++i) {
      need.acquire();
      std::cout << name << " smoker made a cigarette\n";
      agent.release();
    }
  };

  std::thread tobacco_smoker(smoker, "tobacco", std::ref(tobacco));
  std::thread paper_smoker(smoker, "paper", std::ref(paper));
  std::thread matches_smoker(smoker, "matches", std::ref(matches));

  for (int round = 0; round < 6; ++round) {
    agent.acquire();
    if (round % 3 == 0) tobacco.release();
    if (round % 3 == 1) paper.release();
    if (round % 3 == 2) matches.release();
  }

  tobacco_smoker.join();
  paper_smoker.join();
  matches_smoker.join();
}
