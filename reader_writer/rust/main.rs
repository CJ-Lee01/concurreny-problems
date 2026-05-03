// Blocking reader-writer implementation:
// RwLock lets the writer update through an exclusive write guard and lets
// multiple readers observe the value through shared read guards.
use std::sync::{Arc, RwLock};
use std::thread;

fn main() {
    let shared = Arc::new(RwLock::new(0));
    {
        let mut value = shared.write().unwrap();
        *value = 42;
        println!("writer stored 42");
    }

    let mut readers = Vec::new();
    for id in 0..3 {
        let shared = Arc::clone(&shared);
        readers.push(thread::spawn(move || {
            let value = shared.read().unwrap();
            println!("reader {id} saw {value}");
        }));
    }

    for reader in readers {
        reader.join().unwrap();
    }
}
