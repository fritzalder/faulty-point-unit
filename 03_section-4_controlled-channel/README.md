# Section 4 - Controlled channel

We provide the following scripts to reproduce the controlled channel attack: 

 1. Two simple test scripts to verify that the attack works in the enclave: `test.sh` to test the hardware mode and `test_sim.sh` to test the simulation mode.
 2. A script to run the actual controlled channel attack called `binary_search.py`. This script executes 1000 runs of the attack and creates an output called results.csv. On an Intel Core i7-8665U CPU, the script runs at a speed of 5 seconds per iteration which nets to a total runtime of about 90 minutes for a complete run.
 3. A script to evaluate a generated csv file and output a plot. You can control the input file to generate plots from different sources, e.g., `eval.py -f results.csv` or `eval.py -f paper_data.csv` to use the raw data used for the paper Figure 4. The eval script outputs a file named `histogram_error.pdf` that is identical to the file `figure-4.pdf` when called with the original paper data.

## Steps to reproduce Figure 4

To reproduce Figure 4, simply run the scripts in order:

```bash
# Compile and run test
./test.sh
# Run 1000 runs of the attack and place the data in results.csv
# This may take 1-2 hours!
./binary_search.py
# The plot is generated into histogram_error.pdf
./plot.py -f results.csv
```
**Note (runtime).** On an Intel Core i7-8665U CPU, the script runs at a speed of 5 seconds per iteration which nets to a total runtime of about 90 minutes for a complete run. One can control how many tests the `binary_search` script is supposed to run with the `-a` flag which is set to 1000 by default (as this is the reported number in the paper). Example: `./binary_search.py -a 5` to just run 5 tests. We use such a reduced set with 5 runs in the Travis build.

**Note (reproducability).** The binary_search script uses a static seed to make the data reproducible.

**Note (paper data).** To simply verify the plot based on the raw paper data, run `./plot.py -f paper_data.csv`.

To run the controlled channel attack in the simulator, simply run the `./test-sim.sh` script first to compile in simulator mode and then run the attack as normal. It will use the compiled simulator binaries without any further changes to be made.

## Sample output

```bash
$ ./test.s
[Compiling]
[===] Enclave [===]
[RM] inc.o inc.unsigned.so inc.so libinc_proxy.a
[RM] inc_t.o inc_u.o inc_t.h inc_t.c inc_u.h inc_u.c
[RM] main.o main
[===] Enclave [===]
[GEN] sgx_edger8r inc.edl
[CC]  inc_t.c (trusted edge)
[CC]  inc.c (core)
[LD]   inc.o inc_t.o -lsgx_trts inc.unsigned.so
[SGN] inc.unsigned.so
[CC]  inc_u.c (untrusted edge)
[AR]   libinc_proxy.a
[CC]  main.c
[LD] main.o -o main
-fPIC -fno-stack-protector -fno-builtin -fno-jump-tables -fno-common -Wno-attributes -g -D_GNU_SOURCE 
###########################################################################################
# Compiled in HARDWARE mode. To instead compile in hardware mode, make with SGX_MODE=HW #
###########################################################################################
[Testing a case causing an error]
Usage: ./main HEX_VALUE_OF_SECRET HEX_VALUE_OF_INPUT
Result = 255
[Testing a case causing no exception]
Secret = 6.51000000000000023e-01 (0x3fe4d4fdf3b645a2)
Input = 3.41793219432756385e-308 (0x1893dbd2624ff4)
0: oops = 2.22507385850724437e-308 [in = 3.41793219432756385e-308]
Result = 0
[Testing a case causing an exception]
Secret = 6.51000000000000023e-01 (0x3fe4d4fdf3b645a2)
Input = 3.41793219432749765e-308 (0x1893dbd2624f6e)
Caught float exception w code=7 at adrs abs=0x7f87d037b14f;
           --> FPE_FLTINV
Result = 1
```

```bash
$ ./binary_search.py
========================================================================

Searching for secret = 1.2
|value| >= 1 - aborting...
Recovered = 1 after 0 invocations
Error = 0.19999999999999996

========================================================================

Searching for secret = 0.623
|value| < 1, continue...
Recovered = 0.6229998312645415 after 1040 invocations
Error = 1.6873545849449556e-07

========================================================================

Searching for secret = 0.001
|value| < 1, continue...
Recovered = 0.0010000000038146973 after 1040 invocations
Error = 3.814697255993815e-12

========================================================================

Searching for secret = 0.987654321
|value| < 1, continue...
Recovered = 0.9876535767704891 after 1040 invocations
Error = 7.442295107962238e-07

========================================================================

Searching for secret = 0.123456
|value| < 1, continue...
Recovered = 0.12345605753847994 after 1040 invocations
Error = 5.7538479938945564e-08

========================================================================

Searching for secret = 0.5000001
|value| < 1, continue...
Recovered = 0.5000009536761354 after 1040 invocations
Error = 8.536761354482891e-07

========================================================================

Searching for secret = 0.4999999
|value| < 1, continue...
Recovered = 0.4999990463275026 after 1040 invocations
Error = 8.536724974139709e-07

[....]
```
