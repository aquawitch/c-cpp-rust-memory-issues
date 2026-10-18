use std::sync::{Arc, atomic::{AtomicI32, Ordering}};
use std::thread;
fn consume(buf: Vec<u8>) { assert_eq!(buf.len(), 64); }
fn get_name() -> String { String::from("temp") }
fn process(error: bool) -> Result<(), &'static str> {
    let _data = vec![0u8; 1024];
    if error { return Err("expected error"); }
    Ok(())
}
fn find_user(id: u32) -> Option<&'static str> {
    if id == 42 { Some("Ada") } else { None }
}
fn main() {
    println!("{}", get_name());
    let s = String::from("hello"); println!("{s}"); drop(s);
    consume(vec![0u8; 64]);
    let mut v = vec![1,2,3,4];
    let additions: Vec<_> = v.iter().filter(|&&x| x == 2).map(|_| 99).collect();
    v.extend(additions); assert_eq!(v, [1,2,3,4,99]);
    // Runtime input prevents a statically provable out-of-bounds example.
    let i: usize = std::env::args().nth(1).unwrap_or_else(|| "10".into())
        .parse().expect("index must be an unsigned integer");
    let arr = [1,2,3];
    match arr.get(i) {
        Some(x) => println!("arr[{i}] = {x}"),
        None => println!("index {i}: out of range"),
    }
    let counter = Arc::new(AtomicI32::new(0));
    let handles: Vec<_> = (0..2).map(|_| {
        let c = Arc::clone(&counter);
        thread::spawn(move || { for _ in 0..100_000 {
            c.fetch_add(1, Ordering::Relaxed);
        } })
    }).collect();
    for h in handles { h.join().unwrap(); }
    assert_eq!(counter.load(Ordering::Relaxed), 200_000);
    assert!(process(true).is_err()); assert!(process(false).is_ok());
    assert_eq!(find_user(42), Some("Ada"));
    match find_user(0) { Some(name) => println!("{name}"), None => println!("not found") }
    println!("Rust safe examples passed");
}
