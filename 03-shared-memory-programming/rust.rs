use std::thread;

const NOWORKERS: u32 = 5;

fn print_hello(id: u32) {
    println!("Thread {} running", id);
}

fn main() {
    let mut handles = vec![];

    for i in 0..NOWORKERS {
        let handle = thread::spawn(move || {
            print_hello(i);
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    println!("All done.");
}