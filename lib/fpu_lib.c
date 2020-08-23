#include "fpu_lib.h"

#define _SSE_ROUND_NEAREST     (0x0000)
#define _SSE_ROUND_DOWN        (0x2000)
#define _SSE_ROUND_UP          (0x4000)
#define _SSE_ROUND_TOWARD_ZERO (0x6000)
#define _SSE_ROUND_MASK        (0x6000)

bool fpu_verbose_mode = false;

int write_log(const char *format, ...)
{
    if(fpu_verbose_mode){    
        va_list args;
        va_start(args, format);

        vprintf(format, args);

        va_end(args);
    }
}
// Convenience function to dump a buffer
void dump_hex(uint8_t *buf, int len)
{
    printf("[0x");
    for (int i=0; i < len; i++)
        printf("%02x", *(buf + i));
    printf("]\n");
}

/************************************************************************/
uint32_t fauly_points_get_mxcsr(void)
{
    uint32_t rv = 0x0;
    asm("stmxcsr %0\n\t" ::"m"(rv):);

    return rv;
}

void fauly_points_set_mxcsr(uint32_t mxcsr)
{
    asm("ldmxcsr %0\n\t" ::"m"(mxcsr):);
}

uint16_t fauly_points_get_fcw(void)
{
    uint16_t rv = 0x0;
    asm("FSTCW %0\n\t" ::"m"(rv):);

    return rv;
}

void fauly_points_set_fcw(uint16_t fcw)
{
    asm("FLDCW %0\n\t" ::"m"(fcw):);
}

uint64_t mmx_dummy_val = 0;

void faulty_points_perform_mmx_attack(void){
    asm("movq %0, %%mm0\n\t"
    "paddd %%mm0,%%mm0\n\t"
        ::"m"(mmx_dummy_val):);
    write_log("MMX attack: Set Tag word to all valid\n");
}
/************************************************************************/

/*
 * Nice printout of the FPU Control Word register
 * See Intel chapter 8.1.5
*/
void fauly_points_print_fpu_control_word(){
    uint16_t num = fauly_points_get_fcw();
    unsigned int size = sizeof(uint16_t);
    unsigned int maxPow = 1<<(size*8-1);
    //write_log("MAX POW : %u\n",maxPow);
    int i=0,j;
    write_log("FPU Control Word:0x%x (", num);
    for(;i<size*8;++i){
        switch (i)
        {
        case 3:
            write_log(" Inf:");
            break;
        case 4:
            write_log(" RC:");
            break;
        case 6:
            write_log(" PC:");
            break;
        case 0:
        case 8:
            write_log(" Res:");
            break;
        case 10:
            write_log(" Exc:");

        default:
            break;
        }
        // print last bit and shift left.
        write_log("%u",num&maxPow ? 1 : 0);
        num = num<<1;
    }
    write_log(" )\n");
}

/*
 * Nice printout of MXCSR, the SSE MXCSR Control and Status Register
 * See Intel chapter 10.2.3
*/
void fauly_points_print_mxcsr(){
    uint32_t mxcsr = fauly_points_get_mxcsr();
    unsigned int size = sizeof(uint32_t);
    unsigned int maxPow = 1<<(size*8-1);
    //printf("MAX POW : %u\n",maxPow);
    int i=0,j;
    write_log("MXCSR:0x%x (", mxcsr);
    for(;i<size*8;++i){
        switch (i)
        {
        case 0:
            write_log(" Res:");
            break;
        case 16:
            write_log(" FtZ:");
            break;
        case 17:
            write_log(" RC:");
            break;
        case 19:
            write_log(" Masks:");
            break;
        case 25:
            write_log(" DaZ:");
            break;
        case 26:
            write_log(" Exc:");
            break;

        default:
            break;
        }
        // print last bit and shift left.
        write_log("%u",mxcsr&maxPow ? 1 : 0);
        mxcsr = mxcsr<<1;
    }
    write_log(" )\n");
}


/**
 * Sets specific bits of the CW defined by bit_mask
 * */
void set_fpu_CW(uint16_t new_val, uint16_t bit_mask){
    uint16_t val = fauly_points_get_fcw();
    val= (val & (~bit_mask)) | (new_val);
    fauly_points_set_fcw(val);
}

/**
 * Sets the FPU Control Word precision bits (bits 8 and 9)
 * Inputs can be _FPU_DOUBLE, _FPU_SINGLE, _FPU_EXTENDED and the reserved bit 0x100
 * */
void x87_set_precision(uint16_t new_pc){
    set_fpu_CW(new_pc << 8, 0x300);
    write_log("Set CW Precision to %x. \n", new_pc);
}

/**
 * Sets the FPU Control Word rounding bits.
 * Rounding control uses the bits 10 and 11
 * */
void x87_set_rounding(uint16_t new_rc){
    set_fpu_CW(new_rc << 10, 0xC00);
    write_log("Set CW Rounding mode to %x. \n", new_rc);
}

/** 
 * Same as above just with mxcsr. Code taken from xmmintrin.h
 * */
void sse_set_rounding(uint32_t new_rc){
    (fauly_points_set_mxcsr((fauly_points_get_mxcsr() & ~_SSE_ROUND_MASK) | (new_rc << 13)));
    write_log("Set SSE Rounding mode to %x. \n", new_rc);

}

/**
 * Initializes the MXCSR and FCW to the values requested by the 
 *  ENV_VAR_FPU_PRECISION and FPU_ROUND environment variables
 * */
void faulty_point_init(){
    write_log("Faulty Point Library: Setting FCW and MXCSR registers...\n");
    
    // First set precision depending on Environment variable SINGLE, DOUBLE or EXTENDED(default)
    unsigned int prec = 0b11;
    char* env_fpu_prec = getenv(ENV_VAR_FPU_PRECISION);
    if(env_fpu_prec != NULL){
        if(!strcmp(env_fpu_prec, "SINGLE")){
            prec = 0b00;
        }
        if(!strcmp(env_fpu_prec, "DOUBLE")){
            prec = 0b10;
        }
        write_log("Faulty Point Library: Setting x87 precision to %x due to %s set to %s..\n", prec, ENV_VAR_FPU_PRECISION, env_fpu_prec);
    } else {
        write_log("Faulty Point Library: Setting x87 precision to default %x due to %s being unset..\n", prec, ENV_VAR_FPU_PRECISION);

    }
    x87_set_precision(prec);

    // Then set precision depending on environment variable TO_NEAREST(default), UP, DOWN or TO_ZERO
    unsigned int mode = 0b00;
    char* env_fpu_round = getenv(ENV_VAR_FPU_ROUND);
    if(env_fpu_round != NULL){
        if(!strcmp(env_fpu_round, "DOWN")){
            mode = 0b01;
        }
        if(!strcmp(env_fpu_round, "UP")){
            mode = 0b10;
        }
        if(!strcmp(env_fpu_round, "TO_ZERO")){
            mode = 0b11;
        }
        write_log("Faulty Point Library: Setting x87 and SSE Rounding mode to %x due to %s set to %s..\n", mode, ENV_VAR_FPU_ROUND, env_fpu_round);
    } else {
        write_log("Faulty Point Library: Setting x87 and SSE Rounding mode to default %x due to %s being unset..\n", mode, ENV_VAR_FPU_ROUND);
    }
    x87_set_rounding(mode);
    sse_set_rounding(mode);

    // Run MMX attack if desired
    char* env_fpu_mmx = getenv(ENV_VAR_MMX);
    if(env_fpu_mmx != NULL && !strcmp(env_fpu_mmx, "TRUE")){
        faulty_points_perform_mmx_attack();
    }

    write_log("Faulty Point Library: done.\n");
}

// Backup values as they were set by the OS
uint16_t FAULTY_POINTS_FCW_BACKUP = 0;
uint32_t FAULTY_POINTS_MXCSR_BACKUP = 0;

/**
 * Reset MXCSR and FCW to values that were set by OS at the start of execution
 * */
void faulty_point_reset(){
    fauly_points_set_fcw(FAULTY_POINTS_FCW_BACKUP);
    fauly_points_set_mxcsr(FAULTY_POINTS_MXCSR_BACKUP);
    // Also reset FPU tag word
    asm("emms\n\t");
}

uint16_t custom_fcw;
uint32_t custom_mxcsr;
bool use_constructor = false;

/**
 * Constructor to be executed before main that this library is injected to
 * */
__attribute__((constructor)) static void startup(void)
{
    // First: Backup the registers
    FAULTY_POINTS_FCW_BACKUP = fauly_points_get_fcw();
    FAULTY_POINTS_MXCSR_BACKUP = fauly_points_get_mxcsr();
    
    /**
     * Only execute constructor if the ENV_VAR_FPU_LIB_ENABLED environment variable 
     * is set (on default, expect that the program will run the 
     * faulty_point_init and faulty_point_deinit functions )
     */
    char* constructor_env = getenv(ENV_VAR_FPU_LIB_ENABLED);
    char* env_fpu_verbose = getenv(ENV_VAR_FPU_VERBOSE);
    /**
     * Only enable verbose mode if ENV_VAR_FPU_VERBOSE exists and set to TRUE
     * */
    if (env_fpu_verbose != NULL && !strcmp(env_fpu_verbose, "TRUE")){ 
        fpu_verbose_mode = true;
    }
    
    // Only set values now if ENV_VAR_FPU_LIB_ENABLED ENABLED is TRUE
    if( constructor_env != NULL && !strcmp(constructor_env, "TRUE")  ){ 
        use_constructor = true;
        
        // Make a nice printout if we are verbose
        if(fpu_verbose_mode){
            write_log("########## Program entry, Constructor... \n");
            // technically we print this always but it is only visible when logging is actually enabled :)
            write_log("Logging enabled, due to %s being %s\n", ENV_VAR_FPU_VERBOSE, env_fpu_verbose); 
            
            write_log("Default settings for FPU CW and MXCSR:\n");
            fauly_points_print_fpu_control_word();
            fauly_points_print_mxcsr();
        }

        // Set the register values
        faulty_point_init();
        
        // Double check that setting worked
        fauly_points_print_fpu_control_word();
        fauly_points_print_mxcsr();
        
        // store these registers to be able to check them in deconstructor
        custom_fcw = fauly_points_get_fcw();
        custom_mxcsr = fauly_points_get_mxcsr();

        write_log("########## Starting program.. \n\n");
    }

}

/**
 * Destructor: Make sanity check to see if FPU changed
 * */
__attribute__((destructor)) static void shutdown(void)
{   
    /**
     * Get CW and compare it to prevously set value
     * */
    if(use_constructor){
        write_log("\n########## Program exit. Deconstructor...\n");
        uint16_t val = fauly_points_get_fcw();
        if(val != custom_fcw){
            write_log("❌ FPU CW changed during execution !!!!!\n");
            write_log("########## FPU CW NOW:\n");
            fauly_points_print_fpu_control_word();
        } else {
            write_log("✅ FPU CW unchanged after constructor. All good.\n");
        }
        uint32_t mxcsr_val = fauly_points_get_mxcsr();
        if(mxcsr_val != custom_mxcsr){
            write_log("❌ SSE MXCSR changed during execution !!!!!\n");
            write_log("########## MXCSR NOW:\n");
            fauly_points_print_mxcsr();
        } else {
            write_log("✅ FPU CW unchanged after constructor. All good.\n");
        }
        // Reset values again to not mess with any OS 
        write_log("Resetting FPU registers again..\n");
        faulty_point_reset();
        write_log("Destructor done. Exiting.\n");
    }
}