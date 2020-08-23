#! /bin/bash

DATE=$(date +"%Y%m%d%H%M")
EVAL_FOLDER=evaluations/eval_base-poc_$DATE

echo "This script will run the evaluation necessary for Table 1 and place it in directory $EVAL_FOLDER. If you just wish to quickly run and see the data in one view, use the normal eval scripts."
read -r -p "Run and store the eval? [Y/n] " input
 
case $input in
    [yY][eE][sS]|[yY])
 echo "Continuing.."
 ;;
    [nN][oO]|[nN])
 echo "Aborting.."
 exit 0
       ;;
    *)
 echo "Invalid input..."
 exit 1
 ;;
esac

mkdir -p $EVAL_FOLDER
echo "Created directory $EVAL_FOLDER"

echo "####################"
echo "Compiling"
make clean
make all
echo "####################"

echo "Running evals..."
echo "SINGLE Precision"
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=SINGLE ./inc >> $EVAL_FOLDER/sse_single_to-nearest.txt
FPU_VERBOSE=TRUE FPU_ROUND=UP FPU_PRECISION=SINGLE ./inc >> $EVAL_FOLDER/sse_single_up.txt
FPU_VERBOSE=TRUE FPU_ROUND=DOWN FPU_PRECISION=SINGLE ./inc >> $EVAL_FOLDER/sse_single_down.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_ZERO FPU_PRECISION=SINGLE ./inc >> $EVAL_FOLDER/sse_single_to-zero.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=SINGLE FPU_MMX=TRUE ./inc >> $EVAL_FOLDER/sse_single_mmx.txt

echo "DOUBLE Precision"
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=DOUBLE ./inc >> $EVAL_FOLDER/sse_double_to-nearest.txt
FPU_VERBOSE=TRUE FPU_ROUND=UP FPU_PRECISION=DOUBLE ./inc >> $EVAL_FOLDER/sse_double_up.txt
FPU_VERBOSE=TRUE FPU_ROUND=DOWN FPU_PRECISION=DOUBLE ./inc >> $EVAL_FOLDER/sse_double_down.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_ZERO FPU_PRECISION=DOUBLE ./inc >> $EVAL_FOLDER/sse_double_to-zero.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=DOUBLE FPU_MMX=TRUE ./inc >> $EVAL_FOLDER/sse_double_mmx.txt

echo "EXTENDED Precision"
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=EXTENDED ./inc >> $EVAL_FOLDER/sse_extended_to-nearest.txt
FPU_VERBOSE=TRUE FPU_ROUND=UP FPU_PRECISION=EXTENDED ./inc >> $EVAL_FOLDER/sse_extended_up.txt
FPU_VERBOSE=TRUE FPU_ROUND=DOWN FPU_PRECISION=EXTENDED ./inc >> $EVAL_FOLDER/sse_extended_down.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_ZERO FPU_PRECISION=EXTENDED ./inc >> $EVAL_FOLDER/sse_extended_to-zero.txt
FPU_VERBOSE=TRUE FPU_ROUND=TO_NEAREST FPU_PRECISION=EXTENDED FPU_MMX=TRUE ./inc >> $EVAL_FOLDER/sse_extended_mmx.txt
 
echo "Done. Look in $EVAL_FOLDER"
