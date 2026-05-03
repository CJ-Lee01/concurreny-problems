// Blocking cigarette smokers implementation:
// Synchronous channels hand each table placement directly to the smoker that can
// use it, and the agent channel waits for that smoker to finish before continuing.
use std::sync::mpsc::{self, SyncSender};
use std::thread;

fn smoker(name: &'static str, rx: mpsc::Receiver<()>, agent: SyncSender<()>) {
    while rx.recv().is_ok() {
        println!("{name} smoker made a cigarette");
        agent.send(()).unwrap();
    }
}

fn main() {
    let (agent_tx, agent_rx) = mpsc::sync_channel(1);
    let (tobacco_tx, tobacco_rx) = mpsc::sync_channel(0);
    let (paper_tx, paper_rx) = mpsc::sync_channel(0);
    let (matches_tx, matches_rx) = mpsc::sync_channel(0);

    agent_tx.send(()).unwrap();
    let smokers = vec![
        thread::spawn({
            let agent = agent_tx.clone();
            move || smoker("tobacco", tobacco_rx, agent)
        }),
        thread::spawn({
            let agent = agent_tx.clone();
            move || smoker("paper", paper_rx, agent)
        }),
        thread::spawn({
            let agent = agent_tx.clone();
            move || smoker("matches", matches_rx, agent)
        }),
    ];

    for round in 0..6 {
        agent_rx.recv().unwrap();
        match round % 3 {
            0 => tobacco_tx.send(()).unwrap(),
            1 => paper_tx.send(()).unwrap(),
            _ => matches_tx.send(()).unwrap(),
        }
    }

    agent_rx.recv().unwrap();
    drop(tobacco_tx);
    drop(paper_tx);
    drop(matches_tx);
    for smoker in smokers {
        smoker.join().unwrap();
    }
}
