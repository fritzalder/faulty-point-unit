# Proof of concept attack on OpenEnclave

Open Enclave were the initial project to address the core issue described in the paper by using `ldmxcsr/cw` instructions. Following this patch, we started to investigate the issue described in the paper. During our investigation, we discovered a subclass of the issue, namely the MMX attack vector.

## Background and affected runtimes

The trusted enclave entry code reset the x87 FPU control word and SSE MXCSR control registers:

 <https://github.com/openenclave/openenclave/commit/efe75044d215d43c2587ffd79a52074bf838368b>

However, in contrast to other runtimes such as the Intel SGX-SDK, Open Enclave (master branch, commit ef97d964aec4887addbe2d62d96f2c224d984612) used `ldmxcsr/cw` instructions, instead of a full `xrstor` (as patched in e.g. in the Intel SGX-SDK):

<https://github.com/openenclave/openenclave/blob/master/enclave/core/sgx/enter.S#L139>

This left a possibility to corrupt enclaved floating point operations (using the x87 FPU) by issuing MMX instructions in untrusted code before enclave entry. As of publication of this artifact, Microsoft has also decided to apply a full `xrstor` instruction to mitigate this here described issue (see below for details on the patch and CVE).

## Attack description

Concretely, the attack is based on certain ABI state expectations. The System V ABI states that:

```text
> The CPU shall be in x87 mode upon entry to a function.   Therefore,
> every function  that  uses the MMX registers  is  required  to  issue
> an emms or femms instruction after using MMX registers, before returning or
> calling another function.
```

The Intel SDM writes that:

```text
> However, because the MMX registers
> are aliased to the x87 FPU register stack, care must be taken when making
> transitions between x87 FPU instructions and MMX instructions to prevent
> incoherent or unexpected results.
> [...]
> When an x87 FPU instruction is executed, the processor assumes that the
> current state of the x87 FPU register stack and control registers is valid
> and executes the instruction without any preparatory modifications to the x87
> FPU state
```

An attack thus proceeds as follows:

 1. Attacker executes an MMX instruction before entering the enclave
 2. --> The CPU sets x87 TOS=0 and TAG=0xffff (all valid)
 3. Enclave executes x87 instructions that load data into the x87 stack
 4. --> The CPU detects an FPU stack overflow and breaks enclave integrity and/or confidentiality:
    * If exceptions are masked? Enclave silently continues with indefinite result (NaN) (integrity corruption)
    * If exceptions are unmasked? The attacker learns that an x87 instruction was executed (e.g., in secret-dependent execution path)

## Proof-of-concept exploit

This folder contains a minimal PoC developed with the unmodified Open Enclave SDK (master branch, commit ef97d964aec4887addbe2d62d96f2c224d984612). The PoC shows that untrusted code could affect the integrity (expected outcome) of x87 floating point operations. Concretely, without executing MMX instructions before ecall entry, the sample enclave correctly computes a floating point multiplication:

```bash
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