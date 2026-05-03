// Blocking dining philosophers implementation:
// Each fork is a Mutex. Philosophers lock forks in numeric order, so there is
// no cycle in the resource acquisition graph and no deadlock.
use std::cmp::{max, min};
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let forks = Arc::new((0..5).map(|_| Mutex::new(())).collect::<Vec<_>>());
    let mut philosophers = Vec::new();

    for id in 0..5 {
        let forks = Arc::clone(&forks);
        philosophers.push(thread::spawn(move || {
            let left = id;
            let right = (id + 1) % 5;
            let first = min(left, right);
            let second = max(left, right);
            let _first_fork = forks[first].lock().unwrap();
            let _second_fork = forks[second].lock().unwrap();
            println!("philosopher {id} ate");
        }));
    }

    for philosopher in philosophers {
        philosopher.join().unwrap();
    }
}
