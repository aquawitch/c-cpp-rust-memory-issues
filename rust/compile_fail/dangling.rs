fn get_name<'a>() -> &'a str { let name = String::from("temp"); &name }
fn main() { println!("{}", get_name()); }
