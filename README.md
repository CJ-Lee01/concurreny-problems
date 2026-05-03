# Classical Concurrency Problems
This repository contains runnable reference solutions for the problems listed in
`Specification.MD`.

## Disclosure:
This repository was generated with Codex. If there are any errors, file an issue.

## Layout

Implementations are organized as:

```text
<problem>/<language>/implementation_file
```

The language/style folders are:

- `[problem]/blocking_cpp/main.cpp`
- `[problem]/nonblocking_cpp/main.cpp`
- `[problem]/go/main.go`
- `[problem]/rust/main.rs`

The problem folders are:

- Barrier
- Producer Consumer
- Reader Writer
- Dining Philosophers
- Barbershop
- FIFO Semaphore
- H2O
- Cigarette Smokers

For example:

```text
barrier/go/main.go
producer_consumer/nonblocking_cpp/main.cpp
cigarette_smokers/rust/main.rs
```

## Run Examples

```powershell
g++ -std=c++20 -pthread barrier/blocking_cpp/main.cpp -o barrier_blocking.exe
.\barrier_blocking.exe

go run barrier/go/main.go

rustc barrier/rust/main.rs -o barrier_rust.exe
.\barrier_rust.exe
```
