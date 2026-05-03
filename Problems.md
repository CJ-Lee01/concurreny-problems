# Problems

This repository contains reference implementations of several classical
concurrency problems. Each problem is implemented in C++20, Go, and Rust to show
how the same coordination idea can be expressed with different synchronization
tools.

## Barrier

A barrier forces a group of threads or goroutines to wait until every member of
the group has reached the same point in execution. No participant may continue
past the barrier until the required number of participants has arrived.

This is useful when a parallel computation has phases, and each phase must finish
for all workers before the next phase begins.

## Producer Consumer

The producer-consumer problem models one or more producers placing work items
into a bounded FIFO buffer while one or more consumers remove and process those
items. The synchronization challenge is to prevent producers from writing into a
full buffer and consumers from reading from an empty buffer.

This problem captures common queue-based systems such as job dispatch, device
buffers, and pipelines.

## Reader Writer

The reader-writer problem controls access to shared data where readers only
observe the data and writers modify it. Multiple readers can safely read at the
same time, but a writer needs exclusive access so that no reader sees a partially
updated state.

The central concern is balancing correctness with fairness so that neither
readers nor writers are starved.

## Dining Philosophers

The dining philosophers problem models several processes competing for pairs of
shared resources. Each philosopher needs both neighboring forks before eating,
then releases them afterward.

The goal is to design a protocol that avoids deadlock, where everyone holds one
resource and waits forever for another, and avoids starvation, where some
participants never get a chance to proceed.

## Barbershop

The sleeping barber, or barbershop, problem models a server with limited waiting
capacity. Customers arrive, wait if a chair is available, or leave if the waiting
room is full. The barber serves waiting customers and sleeps when there is no
work.

This problem demonstrates coordination between a producer of requests and a
single service worker with bounded queue capacity.

## FIFO Semaphore

A FIFO semaphore limits how many workers can enter a critical section at the
same time, while also granting access in arrival order. This strengthens a
regular semaphore by adding fairness: a worker that has waited longer should be
served before newer arrivals.

The problem focuses on preventing starvation when many threads compete for a
small number of permits.

## H2O

The H2O problem coordinates hydrogen and oxygen threads so that they are released
only in groups that can form one water molecule. Exactly two hydrogen threads and
one oxygen thread must proceed together before the next molecule can form.

It demonstrates how to combine resource counting with a barrier-like rendezvous.

## Cigarette Smokers

The cigarette smokers problem has an agent that places two of three resources on
a table: tobacco, paper, and matches. Each smoker has an infinite supply of one
resource and waits for the other two. The smoker who can complete a cigarette
takes the resources, smokes, and then signals the agent to continue.

This problem models resource allocation and signaling when different workers are
waiting for different combinations of resources.
