fn main() {
    let arr = [1, 2, 3];
    let i: usize = std::env::args().nth(1).unwrap_or_else(|| "10".into())
        .parse().expect("unsigned index");
    println!("{}", arr[i]); // In bounds: success. Out of bounds: panic, not UB.
}
