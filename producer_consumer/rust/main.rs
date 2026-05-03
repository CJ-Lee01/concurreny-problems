// Blocking producer-consumer implementation:
// A sync_channel is the bounded FIFO. The producer blocks when the channel is
// full, and the consumer receives until the producer drops the sender.
use std::sync::mpsc;
use std::thread;

fn main() {
    let (tx, rx) = mpsc::sync_channel(2);
    let producer = thread::spawn(move || {
        for value in 1..=5 {
            tx.send(value).unwrap();
        }
    });
    let consumer = thread::spawn(move || {
        while let Ok(value) = rx.recv() {
            println!("consumed {value}");
        }
    });

    producer.join().unwrap();
    consumer.join().unwrap();
}
