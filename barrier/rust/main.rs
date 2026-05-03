// Blocking barrier implementation:
// std::sync::Barrier parks each worker until all three have called wait, then
// releases the group so they can continue past the rendezvous point.
use std::sync::{Arc, Barrier};
use std::thread;

fn main() {
    let barrier = Arc::new(Barrier::new(3));
    let mut workers = Vec::new();

    for id in 0..3 {
        let barrier = Arc::clone(&barrier);
        workers.push(thread::spawn(move || {
            println!("worker {id} before barrier");
            barrier.wait();
            println!("worker {id} after barrier");
        }));
    }

    for worker in workers {
        worker.join().unwrap();
    }
}
