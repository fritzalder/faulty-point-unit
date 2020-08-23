#include <stdio.h>
#include <sgx_urts.h>
#include <sgx_error.h>
#include "Enclave/inc_u.h"
#include "fpu_lib.h"

sgx_status_t sgx_rv;

#define ENCLAVE_PATH                "./Enclave/inc.so"
#define ENCLAVE_DBG                 1

/**
 * Convenience wrapper to make an ecall.
 * Directly before and after the ecall, call the FPU library in ../lib/ that
 * performs the FPU modifications depending on environment variables.
 * This makes it easier to change results without recompiling the binary every
 * time a different FPU mode wants to be tested.
 * 
 * Importantly, this wrapper sets the FPU mode directly before an ecall and 
 * RESETS it directly after. This is crucial as otherwise the attacker poisons
 * her own FPU mode which has an impact on simple functions such as printf.
 * */
#define SGX_ASSERT(f)                                                       \
    do{                                                                     \
        faulty_point_init();                                                \
        if ( SGX_SUCCESS != (sgx_rv = (f)) )                                \
        {                                                                   \
            printf( "Error calling enclave at %s:%d (rv=0x%x)\n", __FILE__, \
                                                    __LINE__, sgx_rv);      \
            abort();                                                        \
        }                                                                   \
        faulty_point_reset();                                               \
    } while (0)
 
sgx_enclave_id_t eid = 0;

/*
 * If required, this function can create a nice printout of the registers
 * inside and outside of the enclave. This helps to validate the attack and patches.
*/
void print_regs(void)
{
    uint64_t mxcsr = 0;
    uint64_t fcw = 0;

    SGX_ASSERT( ecall_getmxcsr(eid, &mxcsr) );
    printf("           --> enclave mxcsr:                 %p\n", mxcsr);
    printf("           --> untrusted mxcsr:               %p\n", fauly_points_get_mxcsr());

    SGX_ASSERT( ecall_getfcw(eid, &fcw) );
    printf("           --> enclave fcw:                   %p\n", fcw);
    printf("           --> untrusted fcw:                 %p\n", fauly_points_get_fcw());
}

int main( int argc, char **argv )
{
    // results
    long double pi=0;
    long double calc_result=0;

    SGX_ASSERT( sgx_create_enclave( ENCLAVE_PATH, ENCLAVE_DBG,
                                    NULL, NULL, &eid, NULL ) );

    // Make two ecalls, one for pi and one for multiplication
    SGX_ASSERT( ecall_acosf(eid, &pi, -1) );

    SGX_ASSERT( ecall_mul(eid, &calc_result) );

    // Print these results in one line for the small_eval script
    printf("%.26Lf \t%.26Lf \n", pi, calc_result);

    return 0;
}
