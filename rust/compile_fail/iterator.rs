fn main() { let mut v = vec![1,2,3,4]; for x in &v { if *x == 2 { v.push(99); } } }
