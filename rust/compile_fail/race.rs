fn main() {
    let mut counter = 0;
    std::thread::scope(|s| {
        s.spawn(|| { counter += 1; });
        s.spawn(|| { counter += 1; });
    });
    println!("{counter}");
}
