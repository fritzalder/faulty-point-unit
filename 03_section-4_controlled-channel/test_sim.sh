#!/bin/bash

echo [Compiling]
make clean
make SGX_MODE=SIM

echo [Testing a case causing an error]
./main 0x3fe4d4fdf3b645a2
echo Result = $?

echo [Testing a case causing no exception]
./main 0x3fe4d4fdf3b645a2 0x1893dbd2624ff4
echo Result = $?

echo [Testing a case causing an exception]
./main 0x3fe4d4fdf3b645a2 0x1893dbd2624f6e
echo Result = $?
