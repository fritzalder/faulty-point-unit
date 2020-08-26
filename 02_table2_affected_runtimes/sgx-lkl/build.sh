#!/bin/sh

echo -------------------------
echo Installing pre-requisites
echo -------------------------

sudo apt-get install make gcc g++ bc python xutils-dev bison flex libgcrypt20-dev libjson-c-dev automake autopoint autoconf pkgconf libtool libcurl4-openssl-dev libprotobuf-dev libprotobuf-c-dev protobuf-compiler protobuf-c-compiler libssl-dev

echo ------------
echo Patching LKL
echo ------------

cd sgx-lkl-legacy
git apply ../poc.patch

echo --------
echo Building
echo --------

make sim
