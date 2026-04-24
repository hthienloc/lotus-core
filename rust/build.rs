fn main() {
    println!("cargo:rustc-link-search=native=../build");
    println!("cargo:rustc-link-lib=static=lotus_engine_core");
    
    // In case we want to use stdc++
    println!("cargo:rustc-link-lib=stdc++");
}
