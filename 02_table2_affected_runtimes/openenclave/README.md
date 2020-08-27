# Proof of concept attack on OpenEnclave

Open Enclave were the initial project to address the core issue described in the paper by using `ldmxcsr/cw` instructions. Following this patch, we started to investigate the issue described in the paper. During our investigation, we discovered a subclass of the issue, namely the MMX attack vector.

## Background and affected runtimes

The trusted enclave entry code reset the x87 FPU control word and SSE MXCSR control registers. We observed, however, that Open Enclave used `ldmxcsr/cw` instructions, instead of a full `xrstor` (as patched in e.g. in the Intel SGX-SDK):

 <https://github.com/openenclave/openenclave/commit/efe75044d215d43c2587ffd79a52074bf838368b>

This left a possibility to corrupt enclaved floating point operations (using the x87 FPU) by issuing MMX instructions in untrusted code before enclave entry. As of publication of this artifact, Microsoft has also decided to apply a full `xrstor` instruction to mitigate this here described issue (see below for details on the patch and associated CVE).

## Building and running the proof-of-concept exploit

This directory contains a minimal PoC developed with the vulnerable Open Enclave SDK. The PoC shows that untrusted code could affect the integrity (expected outcome) of x87 floating point operations. Concretely, without executing MMX instructions before ecall entry, the sample enclave correctly computes a floating point multiplication:

### 1. Install vulnerable unmodified OpenEnclave SDK

Minimal instructions based on `openenclave/docs/GettingStartedDocs/install_oe_sdk-Simulation.md` and ` openenclave/docs/GettingStartedDocs/install_oe_sdk-Ubuntu_18.04.md`:

```bash
$ sudo dpkg -i --force-depends ~/Downloads/Ubuntu_1804_open-enclave_0.10.0_amd64.deb 
$ cp -r /opt/openenclave/share/openenclave/samples mysamples
$ cd mysamples/helloworld/
$ make simulate 
host/helloworldhost ./enclave/helloworldenc.signed --simulate
Running in simulation mode
Hello world from the enclave
Enclave called into host to print: Hello World!
```
$ patch -p1 < oe.patch
$ make simulate 
host/helloworldhost ./enclave/helloworldenc.signed --simulate
Running in simulation mode
Running in sim mode
Hello world from the enclave
Result = 0.246914



```bash
$ cd openenclave-master/
$ git submodule init
$ git submodule update
$ mkdir build && cd build
$ cmake .. -DHAS_QUOTE_PROVIDER=OFF -DENABLE_REFMAN=OFF -DCMAKE_BUILD_TYPE=Release -DUSE_LIBSGX=OFF
```

### 2. Run modified hello-world application

```bash
$ cd openenclave-master/
$ git apply ../oe.patch


Hello world from the enclave
Running in HW mode
Result = 0.246914
```

When issuing an MMX operation before invocation of the ecall in the untrusted host program, the floating point result is corrupted:

```bash
Hello world from the enclave
Running in HW mode
Result = -nan
```

## Disclosure and mitigation

This issue was acknowledged by Microsoft and assigned CVE-2020-15107 (OpenEnclave security advisory [GHSA-7wjx-wcwg-w999](https://github.com/openenclave/openenclave/security/advisories/GHSA-7wjx-wcwg-w999)). Our proposed mitigation of using an `xrstor` was addressed and incorporated in version 0.10.0.
