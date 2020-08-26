extern crate cc;

fn main() {
    cc::Build::new()
        .file("src/fpu-lib.S")
        .compile("my-fpu-lib");
}
