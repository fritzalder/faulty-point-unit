all: fpuLib

# Lib for FPU
fpuLib: fpu_lib.o
	ar rcs libfpu_lib.a fpu_lib.o

fpu_lib.o : fpu_lib.c
	gcc -o fpu_lib.o -c fpu_lib.c

clean:
	rm fpu_lib.o libfpu_lib.a