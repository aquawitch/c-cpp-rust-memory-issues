fn consume(_: Vec<u8>) {}
fn main() { let buf = vec![0u8;64]; consume(buf); consume(buf); }
