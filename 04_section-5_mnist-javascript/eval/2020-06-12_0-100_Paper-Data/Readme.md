Step 1: Compile sgx-duktape with --mfpmath=387 : $make all
Step 2: Perform first evaluations: $./run_tests.py -x ../sgx-duktape/app -e ../sgx-duktape/enclave.signed.so
Step 3: Remove --mfpmath=387 from Makefile and compile again.
Step 4: Perform second set of evaluations with sse_ prefix: ./run_tests.py -x ../sgx-duktape/app -e ../sgx-duktape/enclave.signed.so -f "sse_"
Step 5: Copy the generated files from both runs into a single folder
Step 6: Run eval script on that folder:
$ ./eval.py -p 2020-06-12_0-100_Paper-Data/
12.06.2020_06:20:24-client_INFO: EVAL RUNNER for SGX+FaaS
12.06.2020_06:20:24-client_INFO: Evaluating files in 2020-06-12_0-100_Paper-Data
12.06.2020_06:20:24-client_INFO: Ignoring filenames ['input.json']
12.06.2020_06:20:24-client_INFO: Directory has files ['sse_SINGLE_TO_NEAREST.log', 'sse_EXTENDED_TO_NEAREST.log', 'sse_EXTENDED_DOWN.log', 'SINGLE_TO_NEAREST.log', 'SINGLE_UP.log', 'SINGLE_TO_ZERO.log', 'SINGLE_DOWN.log', 'EXTENDED_UP.log', 'EXTENDED_DOWN.log', 'sse_SINGLE_UP.log', 'EXTENDED_TO_ZERO.log', 'sse_SINGLE_TO_ZERO.log', 'sse_SINGLE_DOWN.log', 'sse_EXTENDED_UP.log', 'sse_EXTENDED_TO_ZERO.log', 'EXTENDED_TO_NEAREST.log']
12.06.2020_06:20:24-client_INFO: Loaded all files.
12.06.2020_06:20:24-client_INFO: Baseline file sse_EXTENDED_TO_NEAREST.log has 100 entries.

Single precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
SINGLE_DOWN             Round to nearest  8        0.08    [0   0   100 0   0   0   0   0   0   0  ]  [9   14  92  10  14  8   9   14  3   11 ]  0.16796397173637958588621188482647994533181190490723
SINGLE_TO_NEAREST       Rounding down     4        0.04    [0   12  14  2   10  32  0   30  0   0  ]  [9   2   6   8   4   24  9   16  3   11 ]  0.17604646652708841325640776176442159339785575866699
SINGLE_TO_ZERO          Rounding up       8        0.08    [0   0   100 0   0   0   0   0   0   0  ]  [9   14  92  10  14  8   9   14  3   11 ]  0.16796387552144440014068038635741686448454856872559
SINGLE_UP               Round to zero     4        0.04    [0   12  14  2   10  32  0   30  0   0  ]  [9   2   6   8   4   24  9   16  3   11 ]  0.17604643409291073630207336009334539994597434997559

Extended precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
EXTENDED_DOWN           Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
EXTENDED_TO_NEAREST     Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000055440635738398909214256067698490
EXTENDED_TO_ZERO        Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636
EXTENDED_UP             Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592

SSE Single precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
sse_SINGLE_DOWN         Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
sse_SINGLE_TO_NEAREST   Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000000000000000000000000000000000000
sse_SINGLE_TO_ZERO      Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636
sse_SINGLE_UP           Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592

SSE Extended precision:
CW                      Rounding mode     Correct  (Rate)  Class count [0..9]                         Class count difference to baseline [0..9]  Average error                                     
sse_EXTENDED_DOWN       Round to nearest  100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000033073340227149340365918548855201274
sse_EXTENDED_TO_NEAREST Rounding down     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000000000000000000000000000000000000000
sse_EXTENDED_TO_ZERO    Rounding up       100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000052415780706544524366858580853979636
sse_EXTENDED_UP         Round to zero     100      1.0     [9   14  8   10  14  8   9   14  3   11 ]  [0   0   0   0   0   0   0   0   0   0  ]  0.00000000000000031452224755957952899803719081583592
