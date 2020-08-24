#!/bin/bash
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $BASE_DIR

make clean
make all SGX_MODE=HW FPU_MODE=x87
echo ""
echo "###############################################"
echo "Built project in Hardware mode and with x87 FPU"
echo "###############################################"
