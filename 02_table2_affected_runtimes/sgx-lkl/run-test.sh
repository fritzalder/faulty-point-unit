#!/bin/bash

echo -----------------
echo Building test app
echo -----------------

pushd sgx-lkl-legacy/apps/helloworld
make

echo ----------------
echo Running test app
echo ----------------

make test > sgx-lkl-poisoned-run.txt

popd

cp sgx-lkl-legacy/apps/helloworld/sgx-lkl-poisoned-run.txt .


