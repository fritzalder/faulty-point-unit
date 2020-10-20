# Proof of concept attack on OpenEnclave

[![build status](https://travis-matrix-badges.herokuapp.com/repos/fritzalder/faulty-point-unit/branches/master/5)](https://travis-ci.org/github/fritzalder/faulty-point-unit)

Open Enclave were the initial project to address the core issue described in the paper by using `ldmxcsr/cw` instructions. Following this patch, we started to investigate the issue described in the paper. During our investigation, we discovered a subclass of the issue, namely the MMX attack vector.

## Background and affected runtimes

The trusted enclave entry code reset the x87 FPU control word and SSE MXCSR control registers. We observed, however, that Open Enclave used `ldmxcsr/cw` instructions, instead of a full `xrstor` (as patched in e.g. in the Intel SGX-SDK):

 <https://github.com/openenclave/openenclave/commit/efe75044d215d43c2587ffd79a52074bf838368b>

This left a possibility to corrupt enclaved floating point operations (using the x87 FPU) by issuing MMX instructions in untrusted code before enclave entry. As of publication of this artifact, Microsoft has also decided to apply a full `xrstor` instruction to mitigate this here described issue (see below for details on the patch and associated CVE).

## Building and running the proof-of-concept exploit

The `hello-fpu` directory contains a minimal proof-of-concept OE application developed with an unmodified vulnerable Open Enclave SDK. The PoC shows that untrusted code could affect the integrity (expected outcome) of x87 floating point operations by silently replacing them with `NaN` values.

**Note (Docker simulator).** Building and installing OpenEnclave dependencies can be tricky. We therefore recommend to use the Docker container provided by the OpenEnclave team with all dependencies pre-installed, as documented [here](https://github.com/openenclave/openenclave/blob/v0.9.0/docs/GettingStartedDocs/Contributors/BuildingInADockerContainer.md). This is the easiest way to run the example exploit in the OE simulator. In our Dockerfile, we replicate their efforts by manually installing old versions of the required debian packages that were necessary for the vulnerable and patched versions of OpenEnclave.

**Note (OE packages).** For convenience, the current directory includes prebuilt Debian packages for both the vulnerable OE v0.9 and the patched OE v0.10. These are _unmodified_ OE packages that can alternatively also be downloaded from the [official OE release page](https://github.com/openenclave/openenclave/releases).

Proceed as follows to run the proof-of-concept FPU poisoning attack on the vulnerable and patched OE versions.

```bash
$ docker run -i -t oeciteam/oetools-full-18.04

# 1. Clone faulty-point-unit repo
root@oe-docker# cd ~
root@oe-docker# git clone https://github.com/fritzalder/faulty-point-unit.git
root@oe-docker# cd faulty-point-unit/02_table2_affected_runtimes/openenclave/
root@oe-docker# git submodule init && git submodule update

# 2. Install vulnerable OE SDK
root@oe-docker# sudo dpkg -i Ubuntu_1804_open-enclave_0.9.0_amd64.deb
root@oe-docker# source /opt/openenclave/share/openenclave/openenclaverc

# 3. Run proof-of-concept FPU poisoning attack in OE simulation mode
root@oe-docker# cd hello-fpu/ && make
root@oe-docker# make simulate
host/helloworldhost ./enclave/helloworldenc.signed --simulate
host/helloworldhost ./enclave/helloworldenc.signed --simulate
Running in simulation mode
Running in sim mode
Hello world from the enclave
Result = -nan

## 4. Install patched OE SDK
root@oe-docker# sudo dpkg -i ../Ubuntu_1804_open-enclave_0.10.0_amd64.deb
root@oe-docker# source /opt/openenclave/share/openenclave/openenclaverc

# 5. Run proof-of-concept FPU poisoning attack in OE simulation mode
root@oe-docker# make clean && make
root@oe-docker# make simulate
host/helloworldhost ./enclave/helloworldenc.signed --simulate
Running in simulation mode
Running in sim mode
Hello world from the enclave
Result = 0.246914
```

## Disclosure and mitigation

This issue was acknowledged by Microsoft and assigned CVE-2020-15107 (OpenEnclave security advisory [GHSA-7wjx-wcwg-w999](https://github.com/openenclave/openenclave/security/advisories/GHSA-7wjx-wcwg-w999)). Our proposed mitigation of using an `xrstor` was addressed and incorporated in version 0.10.0.
