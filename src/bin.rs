extern crate ebus;
use ebus::root as pleora;
use ebus::scan;
use ebus::Device;
use std::io::prelude::*;
use std::fs::*;
use std::path::Path;
use std::time::{Duration,Instant};

// TODO: add ints for error returns in C instead of voids
// TODO: acquire copy to a byte array
fn main() {

    const TIMEOUT: usize = 1000;
    const BUFFERS: usize = 5;


    if let Some(connection) = scan(TIMEOUT) {
        println!("scan found: {:?}", connection);
        if let Some(mut device) = Device::new(connection, TIMEOUT, BUFFERS) {
            if device.is_active() {
                println!("connection [LIVE]");

                let outcome = device.configure("PixelFormat", "Mono16");
                println!("parameter set success: {}", outcome);

                // tick frequency
                println!("tick: {}", device.tick_frequency());

                // start acquisition
                device.stream();

                // setup output file
                let path = Path::new("outputfile.dat");
                if let Ok(mut f) = File::create(&path) {

                    // acquisition
                    let mut vec: Vec<u8> = vec![0u8; 655360];

                    // start time
                    let start = Instant::now();
                    let mut j = 0;
		    while j < 10_000 {
                    //for i in 0..100 {
                        if device.acquire(&mut vec[..], 1000_i32) != 0 {
                            j+=1;
                            f.write_all(&vec[..]);
                        }
                    }
                    println!("{}", j as f64 / start.elapsed().as_secs() as f64);
                }
            }

        } // <-- implicit shutdown on Drop

    }

    println!("program complete");
}
