#! /bin/bash

# This runs the attack on the inc application in all variations.
# Note, that all 4 rounding and all 3 precision modes can be 
# controlled via the included library fpu_lib which is in the lib folder 
# and which uses environment variables. The MMX mode is controlled via its
# own environment variable and is only activated when that is set to TRUE.
make clean
make all SGX_MODE=SIMULATOR
echo ""
echo      "---------------------------------------------------------------------------"
echo "SINGLE PRECISION"
echo  -e  "Rounding  | arccos(-1) = pi                   | 2.1 * 3.4 = 7.14"
echo      "---------------------------------------------------------------------------"
echo -n "Nearest:    "
FPU_PRECISION=SINGLE FPU_ROUND=TO_NEAREST ./inc
echo -n "Down:       "
FPU_PRECISION=SINGLE FPU_ROUND=DOWN ./inc
echo -n "Up:         "
FPU_PRECISION=SINGLE FPU_ROUND=UP ./inc
echo -n "To Zero:    "
FPU_PRECISION=SINGLE FPU_ROUND=TO_ZERO ./inc
echo -n "MMX attack: "
FPU_PRECISION=SINGLE FPU_ROUND=TO_NEAREST FPU_MMX=TRUE ./inc

echo      "---------------------------------------------------------------------------"
echo      "DOUBLE PRECISION"
echo  -e  "Rounding  | arccos(-1) = pi                   | 2.1 * 3.4 = 7.14"
echo      "---------------------------------------------------------------------------"
echo -n "Nearest:    "
FPU_PRECISION=DOUBLE FPU_ROUND=TO_NEAREST ./inc
echo -n "Down:       "
FPU_PRECISION=DOUBLE FPU_ROUND=DOWN ./inc
echo -n "Up:         "
FPU_PRECISION=DOUBLE FPU_ROUND=UP ./inc
echo -n "To Zero:    "
FPU_PRECISION=DOUBLE FPU_ROUND=TO_ZERO ./inc
echo -n "MMX attack: "
FPU_PRECISION=DOUBLE FPU_ROUND=TO_NEAREST FPU_MMX=TRUE ./inc

echo      "---------------------------------------------------------------------------"
echo "EXTENDED PRECISION"
echo  -e  "Rounding  | arccos(-1) = pi                   | 2.1 * 3.4 = 7.14"
echo      "---------------------------------------------------------------------------"
echo "Baseline/"
echo -n "Nearest:    "
FPU_PRECISION=EXTENDED FPU_ROUND=TO_NEAREST ./inc
echo -n "Down:       "
FPU_PRECISION=EXTENDED FPU_ROUND=DOWN ./inc
echo -n "Up:         "
FPU_PRECISION=EXTENDED FPU_ROUND=UP ./inc
echo -n "To Zero:    "
FPU_PRECISION=EXTENDED FPU_ROUND=TO_ZERO ./inc
echo -n "MMX attack: "
FPU_PRECISION=EXTENDED FPU_ROUND=TO_NEAREST FPU_MMX=TRUE ./inc
