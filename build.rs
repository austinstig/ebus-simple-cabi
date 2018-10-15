extern crate bindgen;
extern crate cc;

use bindgen::builder;
use std::env;
use std::path::PathBuf;

const PLEORA_ROOT: &'static str = "/opt/pleora/ebus_sdk/Ubuntu-x86_64";
const WRAPPER_ROOT: &'static str = "/home/tflight/Documents/ebus-simple-cabi";

fn main() {

    println!("wrapper_root: {:?}", WRAPPER_ROOT);
    // check OS
    if !cfg!(target_os = "linux") {
        eprintln!("only works on linux!");
        return;
    }

    // generate bindings
    let out_dir = env::var("OUT_DIR").unwrap();
    let dest = &format!("{}/wrapper.rs", out_dir);
    let bindings = builder()
        .header(format!("{}/ebus-wrapper/wrapper.h", WRAPPER_ROOT))
        .clang_arg(format!("-I{}/ebus-wrapper", WRAPPER_ROOT))
        .clang_arg(format!("-I{}/include", PLEORA_ROOT))
        .clang_arg(format!("-L{}", WRAPPER_ROOT))
        .clang_arg("-L/lib")
        .whitelist_type("Buffer")
        .whitelist_type("EBUSState")
        .whitelist_function("w_find")
        .whitelist_function("w_connect")
        .whitelist_function("w_configure")
        .whitelist_function("w_tick_frequency")
        .whitelist_function("w_begin_streaming")
        .whitelist_function("w_shutdown")
        .whitelist_function("w_acquire")
        .whitelist_function("w_is_active")
        .whitelist_function("mkbuffer")
        .whitelist_function("mkstate")
        .enable_cxx_namespaces()
        .rustfmt_bindings(true)
        .generate()
        .expect("Failed To Generate Bindings!");
    
    bindings.write_to_file(&dest)
            .expect(&format!("Could not write bindings to: {}", &dest));
    
    // link ebus wrapper
    println!("cargo:rustc-link-search=native={}", WRAPPER_ROOT);
    println!("cargo:rustc-link-lib=dylib={}", "wrapper");

    // link ebus sdk
    println!("cargo:rustc-link-search=native={}",
             "/opt/pleora/ebus_sdk/Ubuntu-x86_64/lib"
    );
    println!("cargo:rustc-link-lib=dylib={}", "PvBase");
    println!("cargo:rustc-link-lib=dylib={}", "PvDevice");
    println!("cargo:rustc-link-lib=dylib={}", "PvStream");
    println!("cargo:rustc-link-lib=dylib={}", "PvBuffer");
    println!("cargo:rustc-link-lib=dylib={}", "PvSystem");
    println!("cargo:rustc-link-lib=dylib={}", "PvPersistence");

    // link to genicam
    println!("cargo:rustc-link-search=native={}",
             "/opt/pleora/ebus_sdk/Ubuntu-x86_64/lib/genicam/bin/Linux64_x64"
    );
    println!("cargo:rustc-link-lib=dylib={}", "PvGenICam");

}