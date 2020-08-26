# Proof of Concept attack on SGX-LKL

We provide two simple scripts to test the poc:

```bash
sudo ./build.sh # Takes a while
sudo ./run.sh
```

**Note (runtime).** Building SGX-LKL may take a long time during the `make sim` target of their build script. On our lab machine, the process took around 30 minutes. Due to this time effort, LKL is also not included in the Travis CI.

## Expected output

The poc is a simple version of a modification of the FCW and MXCSR inside the enclave. After execution of `./run.sh`, the file `sgx-lkl-poisoned-run.txt` is generated which should contain the following lines:

```text
../../build/sgx-lkl-run sgxlkl-disk.img app/helloworld
MXCSR = 00003fa1
FCW = 077f
rint(11.5) = 11.000000
acosf(-1) = 3.1415925025939941406250
MXCSR = 00003fa1
FCW = 077f

```

Where the two lines in the middle show the results of a rounding and a calculation of pi inside LKL and the outer two lines each show the value of the MXCSR and FCW registers.

The file `sgx-lkl-poisoned-run_reference-output.txt` is a reference output of the same script execution for the reviewer's convenience.

## Disclosure and mitigation

SGX-LKL is currently in the process of migration their shielding runtime to Microsoft OpenEnclave which will make them inherit the patch applied there. As such, no mitigation is currently needed by SGX-LKL.