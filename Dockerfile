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
