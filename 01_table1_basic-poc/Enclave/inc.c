#include <stdint.h>
#include <math.h>


/**
 * Preparing the values here as global variables ensures
 * that the compiler does not perform optimizations such as
 * directly calculating the result. We want the
 * executed code to load the actual variable into the FPU
 * since this is where the attack happens.
 * */
uint64_t volatile g_rv = 0;
long double a = 2.1;
long double b = 3.4;

/*
 * Convenience functions to get the MXCSR and FCW values from inside the enclave.
*/
int ecall_getmxcsr(void)
{
    g_rv = 0;
    asm("lea g_rv(%%rip), %%rax\n\t" \
        "stmxcsr (%%rax)\n\t":::"rax");

    return g_rv;
}

int ecall_getfcw(void)
{
    g_rv = 0;
    asm("lea g_rv(%%rip), %%rax\n\t" \
        "FSTCW (%%rax)\n\t":::"rax");

    return g_rv;
}

/**
 * Actual functions that calculate the Table 1 results.
 * By letting the enclave directly write to the pointer,
 * we ensure that it is not the untrusted part that accidentally
 * degrades the precision of the number when it receives the variable.
 * */
void ecall_acosf(long double *rv, int a)
{
    *rv = acosl(a);
}

void ecall_mul(long double *rv)
{
    *rv= a*b;
}