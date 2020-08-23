# Faulty Point Unit: ABI Poisoning Attacks on Intel SGX

This is a repository to reproduce and view the source code and data used in the paper "Faulty Point Unit: ABI Poisoning Attacks on Intel SGX" to appear at ACSAC '20:
```
Faulty Point Unit: ABI Poisoning Attacks on Intel SGX
Authors: Fritz Alder*, Jo Van Bulck*, David Oswald+, Frank Piessens*
  * imec-DistriNet, KU Leuven, Belgium
  + University of Birmingham, UK
```

## Artifact description

In this artifact, we provide means to reproduce all of our experimental results.

More specifically, following the tables and figures in the paper:
| Paper reference | Directory | Description |
| :---- | :-------- | :---------- |
| Table 1 | `01_table1_basic-poc` | Proof-of-concept poisoning attack executed against an Intel SGX-SDK enclave. We provide the documented source code, build instructions, and the raw paper data.|
| Table 2 | `02_table2_affected_runtimes` | List of affected enclave shielding runtimes. We provide proof-of-concept poisoning attacks against all runtimes marked with a star. Namely, OpenEnclave, SGX-LKL, Rust-EDP, and Go-TEE (for Intel SGX-SDK, see the artifact for Table 1).|
| Figure 4 | `03_section-4_controlled-channel` | Controlled channel attack. We provide the documented proof-of-concept attacker and victim source code, build instructions, and the raw paper data.|
| Table 3 | `04_section-5_mnist-javascript` | Machine learning with Javascript inside an enclave. We provide the documented source code, build instructions, and the raw paper data. This example was built on the open-source code of the S-FaaS project.|
| Table 4 | `05_section-6_spec-cpu-2017` | Benchmarks with SPEC CPU 2017. Due to licensing and copyright restrictions with the proprietary SPEC 2017 suite, we can only provide the used configuration file and detailed instructions how to reproduce our results for people who already bought the SPEC CPU 2017 suite. Please note however, that obtaining the raw data for the SPEC benchmarks summarized in Table 4 takes several CPU weeks. For the artifact evaluation, we therefore do *not* expect artifact reviewers to generate the raw data themselves. We instead provide the log outputs of the full SPEC runs with detailed instructions of how to reproduce the summary in Table 4.|
| Figure 6 | `06_figure-6_blender-outputs` | Screenshot of the Blender benchmark. We provide the full reference output of Blender in normal conditions and under the attack.|


All of the attacks described in our paper were performed on real SGX hardware. For the convenience of the artifact evaluation however, we ensured that all of our main results can also run in the official Intel SGX simulator, i.e., on normal processors without Intel SGX support. We further provide an installation script and a Docker container with all the dependencies preinstalled. Note, that simulation mode does not affect our attacks as we exploit software sanitization oversights that occur in code that is agnostic to whether the enclave executes on real hardware or in a simulator.

Further note, that not all of the enclave shielding runtimes that we studied, e.g., Rust-EDP offer a simulation mode and we hence are not able to offer a simulation mode for all of the attacks. This is only a concern for Table 2. To test the artifacts in a real hardware environment we also provide SSH access to a preinstalled SGX-enabled lab machine.

## Preparation and setup
This artifact demonstrates an attack that existed in the Intel SGX SDK in version 2.7.1 and was patched following with version 2.8.0. As such, we provide an easy to use install script that allows to reproduce the attack in both versions and be able to verify its mitigation in the patched SDK version.

We provide the script `sdk_helper.sh` to install and switch between the two relevant Intel SGX SDK versions. Simply run `./sdk_helper.sh install` to install the relevant dependencies in hardware mode or `./sdk_helper install_sim` to install the dependencies in simulation mode if you have no intention of running the attack on real hardware. As described earlier, since this attack exploits software sanitization oversights, the simulation yields the same results when reproducing the data as real hardware will.

The `sdk_helper.sh` script has two options to switch between Intel SGX SDK versions: `sdk_helper.sh vulnerable` and `sdk_helper.sh vulnerable` to switch to the Intel SGX SDK 2.7.1 and 2.8 respectively. **Note** that in order for the script to perform this switch reliably, this script needs to be called with `source` so that the environment variables that are set are inherited by your shell. As such, always call the script as `source sdk_helper.sh vulnerable` and `source sdk_helper.sh vulnerable`!

A quick start example of commands to start with this artifact is as follows:
```bash
# Install dependencies
./sdk_helper install_sim
# Switch to 2.7.1
source ./sdk_helper vulnerable
# Reproduce basic proof of concept attack
cd 01_table1_basic-poc
./eval_simulator.sh
# Now reproduce same attack with patched SDK
source ../sdk_helper patched
./eval_simulator.sh
```

Use `sdk_helper help` to print an overview of commands and tasks the script performs.

## Intel SGX SDK bug in Ubuntu 20.04LTS
As of release of this artifact, there is a known issue with the Intel SGX SDK and Ubuntu LTS 20.04 as reported [here](https://lore.kernel.org/linux-sgx/4db41057-910c-b686-0428-474debe382c1@fortanix.com/), [here](https://github.com/intel/linux-sgx/issues/569), and [here](https://github.com/intel/linux-sgx/pull/515). Since we are deliberately installing an obsolete Intel SGX SDK to make it vulnerable to a known attack, we can not rely on future releases to patch this issue. A quick workaround is to set `mount -o remount,exec /dev` to circumvent the new behavior of Ubuntu 20.04. The `sdk_helper` sets this before installing if it detects the mentioned Ubuntu version. We **explicitly** warn of possible side-effects. Additionally, since this setting resets on reboot, a user might have to simply rerun the installation with the `sdk_helper` after a reboot.
