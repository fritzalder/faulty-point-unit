# MNist predictions with Javascript in an enclave

The second case study in the paper performs a machine learing prediction in an enclave that runs the `Convnet.JS` Javascript library ([<https://cs.stanford.edu/people/karpathy/convnetjs/>]). The source code for the enclave and its Javascript runtime is adopted from earlier research:

```bibtex
S-FaaS: Trustworthy and Accountable Function-as-a-Service using Intel SGX
Authors: Fritz Alder*$, N. Asokan*, Arseny Kurnikov*, Andrew Paverd*, Michael Steiner+
 * Aalto University
 + Intel Labs
 $ Also first author of this paper.
GitHub repository: https://github.com/SSGAalto/sfaas
```

As described in the paper, we adopt the source code of `S-FaaS` to only execute a single Javascript file given as input inside the enclave. As such, we removed all of the networking code and stripped all encryptions of inputs and outputs. The result is a smaller application that retrieves a Javascript file to execute and a JSON file as its input.

## JavaScript files

Convnet.JS is a research project that brings convolutional neural networks to JavaScript ([<https://cs.stanford.edu/people/karpathy/convnetjs/>]). The sgx duktape enclave requires two inputs: The convnet.js Javascript file and a JSON file as input that contains the neural network to be executed and an input to evaluate. See the subdirectory `js-files` for more information.

## Quick usage of sgx-duktape

To quickly verify that the sgx-duktape enclave works, we provide a small MNist input with only 5 images. Run this with the following commands:

```bash
cd sgx-duktape
source ../../sdk_helper.sh vulnerable
./compile_fpu_simulator.sh
./app -x ../js-files/convnet.js  -i ../js-files/mnist-network-with-input-only5.json
```

or alternatively to run in hardware mode, this time with the full 100 images that were used in the paper evaluation:

```bash
cd sgx-duktape
source ../../sdk_helper.sh vulnerable
./compile_fpu_hw.sh
./app -x ../js-files/convnet.js  -i ../js-files/mnist-network-with-input.json
```

The generated output is a raw JSON string that is not very human readable. We provide two scripts in the `eval` folder to make running and evaluating the results more easy.

## Reproducing Table 3

![Table3 screenshot](table3.png)

To reproduce Table 3, one has to perform two sets of 8 exeuctions each of the sgx duktape enclave: One compilation with the x87 FPU and two precision and four rounding modes, and one compilation with the default SSE version. The script `run-tests.py` performs all these runs automatically and places them in a new subidectory:

```bash
cd eval
source ../../sdk_helper.sh vulnerable
./run_tests.py --simulator
# Alternatively, calling the script without --simulator would run this test in hardware mode

# Now a directory was created. Show a Table similar to Table 3 with:
./eval.py -p 2020-06-12_0-100_Paper-Data
# where 2020-06-12_0-100_Paper-Data would be the freshly created folder.
```

**Note (runtime).** A full run of the `run_tests.py` script takes around 10 minutes on our Intel Core i7-8665U CPU.

To verify the mitigation, one can run  `source sdk_helper.sh patch` and rerun the lines above (without the `source` command) which should yield correct results independent of the used Rounding or Precision modes.
