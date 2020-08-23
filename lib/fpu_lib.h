#ifndef FPU_LIB_H
#define FPU_LIB_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

// General flag to enable the constructor of the library
// If set to TRUE , the library sets the FPU state before program
// execution so that the program does not have to be aware of the library.
#define ENV_VAR_FPU_LIB_ENABLED "FPU_LIB_ENABLED" 

// Precision settings for the FCW
// Accepted values are SINGLE DOUBLE and EXTENDED
#define ENV_VAR_FPU_PRECISION "FPU_PRECISION" 

// Rounding mode for both FCW and MXCSR
// Accepted values are UP DOWN TO_NEAREST and TO_ZERO
#define ENV_VAR_FPU_ROUND "FPU_ROUND"   

// Flag whether the MMX attack should be performed
// Accepted values are only TRUE. Disabled otherwise
#define ENV_VAR_MMX "FPU_MMX"

// FPU verbose mode if set to TRUE
#define ENV_VAR_FPU_VERBOSE "FPU_VERBOSE" 

// Initialize and reset the faulty point attack
void faulty_point_init();
void faulty_point_reset();

// Functions to get and set the registers based on environment variables
uint32_t fauly_points_get_mxcsr(void);
void fauly_points_set_mxcsr(uint32_t mxcsr);
uint16_t fauly_points_get_fcw(void);
void fauly_points_set_fcw(uint16_t fcw);

// Functions for convenience printouts
void fauly_points_print_fpu_control_word();
void fauly_points_print_mxcsr();

// Convenience function to dump print a buffer in hex
// This ruleso ut any FPU issues that printf might add
void fauly_points_dump_hex(uint8_t *buf, int len);

#endif