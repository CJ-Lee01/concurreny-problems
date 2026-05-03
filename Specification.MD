# Specification

This respository is meant to be a collection of solutions for the classical concurrency problems:
| Problem | Description |
| - | - | 
| Barrier | Wait until threads/processes reach a specific point in the execution |
| Producer Consumer | Model interactions between a processor and devices that interact through FIFO channels | 
| Reader Writer | Model access to shared memory |
| Dining Philosophers | Allocation of limited resources to a group of processes in a deadlock-free and starvation-free manner |
| Barbershop | Coordinating the execution of a processor |
| FIFO Semaphore | Avoid starvation and increase fairness in the system |
| H2O | Allocation of specific resource to a process |
| Cigarette Smokers | The agent represents an operation system that allocates resources, and the smokers represent applications that need resources | 

The solution for each problem is to be implemented in 4 different styles:
- Blocking C++20 (e.g. using std::barrier, std::mutex)
- Nonblocking C++20 (e.g. using compare and swap)
- Go goroutines and channels (and sync.WaitGroup if required)
- Rust 

Directory structure: ./problem/language/implementation_file
