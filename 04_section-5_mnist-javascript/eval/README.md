# Evaluation

There are two evaluation scripts. One to run the tests and create outputs and one to evaluate those outputs. The run script takes the following arguments:
| Argument | Default | Description |
| :------- | :------ | :---------- |
| `--javascript` | `../js-files/convnet.js` | Defines the Javascript file to execute |
| `--neural-net` | `../js-files/trained-network.json` | Defines the neural net that is used as first input to the Javascript file. |
| `--mnist` | `../js-files/mnist_handwritten_test_first100.json` | Defines the mnist test images that are to be evaluated on the neural net. |
| `--amount` | `100` | The number of images that are picked from the mnist input.|
| `--initial` | `0` | The initial index to start from at the mnist input. This allows to run the evaluation in batches.|
| `--simulator` | Not set | Sets the enclave to run in simulator mode instead of hardware mode.|


## run_tests.py
The `run_test` script performs multiple executions of the sgx-duktape enclave application for each desired setting of the FPU and stores the outputs in a subdirectory. These outputs can then be evaluated in the next step by the `eval` script.

To run the tests, simply call `run_tests.py` with default parameters. The script automatically compiles the sgx-duktape enclave twice: Once with the x87 FPU and once with SSE. Since these benchmarks may take a while, it is also possible to just run a smaller batch first, e.g., with an `--amount=5` option to verify that the system is set up correctly. On an Intel(R) Core(TM) i7-8665U CPU, the run_test script with default values takes around 12 minutes in either hardware or simuation mode. Since 8 processes are spawned in parallel two times, your results may vary depending on the amount of cores you have available.

```bash
./run_tests.py --simulator                                                                                                               Mon 24.08.2020_12:55:17-client_INFO: EVAL RUNNER for SGX+FaaS
24.08.2020_12:55:17-client_INFO: Created directory 2020-08-24_12-55-17_0-100_SIM-RUN
24.08.2020_12:55:17-client_INFO: Using 2020-08-24_12-55-17_0-100_SIM-RUN as base directory for outputs
24.08.2020_12:55:17-client_INFO: Compiling enclave first in x87 mode..
[... compilation ...]
24.08.2020_12:40:32-client_INFO: Starting first round of tests
24.08.2020_12:40:32-client_INFO: Executing for inputs: [('SINGLE', 'UP'), ('SINGLE', 'DOWN'), ('SINGLE', 'TO_NEAREST'), ('SINGLE', 'TO_ZERO'), ('EXTENDED', 'UP'), ('EXTENDED', 'DOWN'), ('EXTENDED', 'TO_NEAREST'), ('EXTENDED', 'TO_ZERO')]
24.08.2020_12:40:32-client_INFO: Starting subprocesses..
100%|█████████████████████████████████████████████████████████████| 8/8 [00:00<00:00, 272.82it/s]
24.08.2020_12:40:32-client_INFO: done. All processes started.
24.08.2020_12:40:32-client_INFO: Waiting for completion...
100%|██████████████████████████████████████████████████████████████| 8/8 [05:40<00:00, 42.51s/it]
24.08.2020_12:46:12-client_INFO: Second step: Compiling enclave in SSE mode..
[ ... compilation ...]
24.08.2020_12:46:17-client_INFO: Starting second round of tests
24.08.2020_12:46:17-client_INFO: Executing for inputs: [('SINGLE', 'UP'), ('SINGLE', 'DOWN'), ('SINGLE', 'TO_NEAREST'), ('SINGLE', 'TO_ZERO'), ('EXTENDED', 'UP'), ('EXTENDED', 'DOWN'), ('EXTENDED', 'TO_NEAREST'), ('EXTENDED', 'TO_ZERO')]
24.08.2020_12:46:17-client_INFO: Starting subprocesses..
100%|█████████████████████████████████████████████████████████████| 8/8 [00:00<00:00, 312.42it/s]
24.08.2020_12:46:17-client_INFO: done. All processes started.
24.08.2020_12:46:17-client_INFO: Waiting for completion...
100%|█████████████████████████████████████████████████████████████| 8/8 [05:47<00:00, 118.52s/it]
24.08.2020_12:52:04-client_INFO: Executed all input settings. Done.

```

Note, that you can specify `--amount=X` for testing to limit the test size.
By default, the input file with 100 inputs is used. To change that to the another version, use the -m option to change the path. See the Readme in `js-files/extra_mnist-preparation` for information on how to generate other MNist inputs.

## eval.py
The eval script allows to evaluate the outputs generated by the `run_tests.py` script and parse them into a human-readable format (as the outputs generated by the Duktape enclave are all in JSON format). To run the `eval` script, simply give the folder to evaluate as `-p` option. The run_test script generates a combination of two runs: One run compiled with the x87 FPU and one run compiled with standard SSE, i.e., the first two groups in Table 3 are compiled with `make FPU_MODE=x87` while the last group (SSE) is simply compiled with `make`. The run_tests script performs all this automatically and places the output files into the same folder so that the eval script can make all data human readable.

```bash
$ ./eval.py -p 2020-06-12_0-100_Paper-Data/
28.08.2020_12:39:27-client_INFO: EVAL RUNNER for SGX+FaaS
28.08.2020_12:39:27-client_INFO: Evaluating files in 2020-06-12_0-100_Paper-Data
28.08.2020_12:39:27-client_INFO: Ignoring filenames ['input.json', 'Readme.md', 'README.md']
28.08.2020_12:39:27-client_INFO: Directory has files ['sse_SINGLE_UP.log', 'sse_SINGLE_TO_NEAREST.log', 'SINGLE_UP.log', 'EXTENDED_TO_NEAREST.log', 'SINGLE_DOWN.log', 'sse_SINGLE_DOWN.log', 'SINGLE_TO_ZERO.log', 'sse_SINGLE_TO_ZERO.log', 'sse_EXTENDED_TO_ZERO.log', 'sse_EXTENDED_DOWN.log', 'sse_EXTENDED_UP.log', 'sse_EXTENDED_TO_NEAREST.log', 'EXTENDED_DOWN.log', 'EXTENDED_TO_ZERO.log', 'SINGLE_TO_NEAREST.log', 'EXTENDED_UP.log']
28.08.2020_12:39:27-client_INFO: Loaded all files.
28.08.2020_12:39:27-client_INFO: Baseline file sse_EXTENDED_TO_NEAREST.log has 100 entries.

Single precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
SINGLE_TO_NEAREST       Round to nearest  4        0.04    [0   12  14  2   10  32  0   30  0   0  ]  [9   2   6   8   4   24  9   16  3   11 ]  0.17604646652708841325640776176442159339785575866699
SINGLE_DOWN             Rounding down     8        0.08    [0   0   100 0   0   0   0   0   0   0  ]  [9   14  92  10  14  8   9   14  3   11 ]  0.16796397173637958588621188482647994533181190490723
SINGLE_UP               Rounding up       4        0.04    [0   12  14  2   10  32  0   30  0   0  ]  [9   2   6   8   4   24  9   16  3   11 ]  0.17604643409291073630207336009334539994597434997559
SINGLE_TO_ZERO          Round to zero     8        0.08    [0   0   100 0   0   0   0   0   0   0  ]  [9   14  92  10  14  8   9   14  3   11 ]  0.16796387552144440014068038635741686448454856872559

Extended precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
EXTENDED_TO_NEAREST     Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000055440635738398909214256067698490
EXTENDED_DOWN           Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
EXTENDED_UP             Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592
EXTENDED_TO_ZERO        Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636

SSE Single precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
sse_SINGLE_TO_NEAREST   Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000000000000000000000000000000000000
sse_SINGLE_DOWN         Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
sse_SINGLE_UP           Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592
sse_SINGLE_TO_ZERO      Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636

SSE Extended precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
sse_EXTENDED_TO_NEAREST Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000000000000000000000000000000000000
sse_EXTENDED_DOWN       Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
sse_EXTENDED_UP         Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592
sse_EXTENDED_TO_ZERO    Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636
```
