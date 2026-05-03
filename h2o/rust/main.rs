// Blocking H2O implementation:
// Counting semaphores built from Mutex and Condvar admit two hydrogens and one
// oxygen. A Barrier groups the three atoms before permits are returned.
use std::sync::{Arc, Barrier, Condvar, Mutex};
use std::thread;

struct CountingSemaphore {
    permits: Mutex<i32>,
    cond: Condvar,
}

impl CountingSemaphore {
    fn new(permits: i32) -> Self {
        Self {
            permits: Mutex::new(permits),
            cond: Condvar::new(),
        }
    }

    fn acquire(&self) {
        let mut permits = self.permits.lock().unwrap();
        while *permits == 0 {
            permits = self.cond.wait(permits).unwrap();
        }
        *permits -= 1;
    }

    fn release(&self) {
        let mut permits = self.permits.lock().unwrap();
        *permits += 1;
        self.cond.notify_all();
    }
}

fn main() {
    let hydrogen = Arc::new(CountingSemaphore::new(2));
    let oxygen = Arc::new(CountingSemaphore::new(1));
    let barrier = Arc::new(Barrier::new(3));
    let mut atoms = Vec::new();

    for _ in 0..4 {
        let hydrogen = Arc::clone(&hydrogen);
        let barrier = Arc::clone(&barrier);
        atoms.push(thread::spawn(move || {
            hydrogen.acquire();
            println!("H ready");
            barrier.wait();
            hydrogen.release();
        }));
    }
    for _ in 0..2 {
        let oxygen = Arc::clone(&oxygen);
        let barrier = Arc::clone(&barrier);
        atoms.push(thread::spawn(move || {
            oxygen.acquire();
            println!("O ready");
            barrier.wait();
            oxygen.release();
        }));
    }

    for atom in atoms {
        atom.join().unwrap();
    }
}
