# Clean Ubuntu 18.04 container
FROM ubuntu:18.04
RUN apt-get update -yqq && apt-get -yqq install git lsb-release sudo vim 

# Config parameters
WORKDIR faulty-point-unit

# Set to noninteractive mode
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Brussels
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install dependencies for SGX-LKL, Go-TEE, Rust-EDP
RUN apt-get install make gcc g++ bc python xutils-dev bison flex libgcrypt20-dev libjson-c-dev autopoint pkgconf autoconf libtool libcurl4-openssl-dev libprotobuf-dev libprotobuf-c-dev protobuf-compiler protobuf-c-compiler libssl-dev rsync golang curl -yqq

# Prepare Docker for OE, see https://github.com/openenclave/openenclave/pull/2027
RUN apt-get -yqq install wget gnupg python apt-transport-https libssl1.1
RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' | tee /etc/apt/sources.list.d/intel-sgx.list && wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -
RUN mkdir -p /etc/init
RUN apt-get update && apt-get install -yqq libsgx-enclave-common=2.3.100.46354-bionic1 libsgx-enclave-common-dev=2.3.100.0-1 libsgx-dcap-ql=1.0.100.46460-bionic1.0 libsgx-dcap-ql-dev=1.0.100.46460-bionic1.0

# Build and install Intel SGX simulator
RUN git clone https://github.com/fritzalder/faulty-point-unit .
RUN ./sdk_helper.sh install_sim

# Display a welcome message for interactive sessions
RUN echo '[ ! -z "$TERM" -a -r /etc/motd ] && cat /etc/motd' \
	>> /etc/bash.bashrc ; echo "\
========================================================================\n\
= Faulty Point Unit SGX simulator Docker container                     =\n\
========================================================================\n\
`lsb_release -d`\n\n\
To get started, see README.md in the current directory\n"\
> /etc/motd

CMD /bin/bash
