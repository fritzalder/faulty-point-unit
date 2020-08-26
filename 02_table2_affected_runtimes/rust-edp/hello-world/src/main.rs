use core::arch::x86_64::*;

extern "C" {
    fn add_fpu(a: f64) -> f64;
    fn get_fpucw() -> u16;
}

fn main() {
    let num = 0.123456789_f64;
    let a = -0.9999999_f64;
    let rv:f64;
    unsafe {
        rv = add_fpu(num);
    }

    println!("[enclave] Hello, world!");
    println!("Pi      = {:.64}", a.acos());
    unsafe {
        println!("MXCSR   = {:#x}", _mm_getcsr());
        println!("FPUCW   = {:#x}", get_fpucw());
    };

    println!("num     = {:.64}", num);
    println!("2*num   = {:.64}", 2_f64 * num);
    println!("add_fpu = {:.64}", rv);
}
