// Blocking barbershop implementation:
// A sync_channel models the finite waiting room. Customers use try_send to sit
// only when a chair is available, while the barber receives queued customers.
use std::sync::mpsc;
use std::thread;

fn main() {
    let (waiting_room_tx, waiting_room_rx) = mpsc::sync_channel::<usize>(2);
    let barber = thread::spawn(move || {
        let mut served = 0;
        while let Ok(customer) = waiting_room_rx.recv() {
            served += 1;
            println!("barber served customer {customer} as {served}");
        }
    });

    let mut customers = Vec::new();
    for id in 1..=5 {
        let tx = waiting_room_tx.clone();
        customers.push(thread::spawn(move || match tx.try_send(id) {
            Ok(()) => println!("customer {id} waited"),
            Err(_) => println!("customer {id} left"),
        }));
    }

    drop(waiting_room_tx);
    for customer in customers {
        customer.join().unwrap();
    }
    barber.join().unwrap();
}
