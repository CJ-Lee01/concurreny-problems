// Blocking FIFO semaphore implementation:
// A Mutex-protected state stores permits and ticket counters. A Condvar wakes
// waiters until the next ticket holder can consume a permit.
use std::sync::{Arc, Condvar, Mutex};
use std::thread;

struct FifoSemaphoreState {
    permits: i32,
    next: usize,
    serving: usize,
}

struct FifoSemaphore {
    state: Mutex<FifoSemaphoreState>,
    cond: Condvar,
}

impl FifoSemaphore {
    fn new(permits: i32) -> Self {
        Self {
            state: Mutex::new(FifoSemaphoreState {
                permits,
                next: 0,
                serving: 0,
            }),
            cond: Condvar::new(),
        }
    }

    fn acquire(&self) {
        let mut state = self.state.lock().unwrap();
        let ticket = state.next;
        state.next += 1;
        while ticket != state.serving || state.permits == 0 {
            state = self.cond.wait(state).unwrap();
        }
        state.permits -= 1;
        state.serving += 1;
        self.cond.notify_all();
    }

    fn release(&self) {
        let mut state = self.state.lock().unwrap();
        state.permits += 1;
        self.cond.notify_all();
    }
}

fn main() {
    let semaphore = Arc::new(FifoSemaphore::new(1));
    let mut workers = Vec::new();

    for id in 0..4 {
        let semaphore = Arc::clone(&semaphore);
        workers.push(thread::spawn(move || {
            semaphore.acquire();
            println!("fifo worker {id}");
            semaphore.release();
        }));
    }

    for worker in workers {
        worker.join().unwrap();
    }
}
