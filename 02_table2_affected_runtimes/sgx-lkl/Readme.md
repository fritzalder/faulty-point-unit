# Proof of Concept attack on SGX-LKL

We provide two simple scripts to test the poc:

```bash
sudo ./build.sh # Takes a while
sudo ./run.sh
```

**Note (runtime).** Building SGX-LKL may take a long time during the `make sim` target of their build script. Patience is required.

## Disclosure and mitigation

SGX-LKL is currently in the process of migration their shielding runtime to Microsoft OpenEnclave which will make them inherit the patch applied there. As such, no mitigation is currently needed by SGX-LKL.
