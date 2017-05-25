extern crate csv;

use std::env;
use std::error::Error;
use std::ffi::OsString;
use std::fs::File;

fn main() {
    let mut count = 0;
    let file_path = get_first_arg().unwrap();
    let file = File::open(file_path).unwrap();
    let mut rdr = csv::Reader::from_reader(file);

    for _ in rdr.records() {
        count += 1;
    }

    println!("{}", count);
}

fn get_first_arg() -> Result<OsString, Box<Error>> {
    match env::args_os().nth(1) {
        None => Err(From::from("expected 1 argument, but got none")),
        Some(file_path) => Ok(file_path),
    }
}
