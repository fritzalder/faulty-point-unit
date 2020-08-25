# Faulty Point library for easy access to the FPU attack

This library provides functions and a constructor hook that can be controlled via environment variables to modify the FPU and SSE registers without needing to recompile the investigated code. This allows for faster experimentation than always needing to provide multiple options to each binary for all combinations of rounding and precision modes.

## Usage from applications
You can simply link in the library from applications. E.g., the basic poc for Table 1 uses this line in the Makefile of the untrusted application: `-L$(CURDIR)/../lib/ -lfpu_lib` and this include `#include "fpu_lib.h"`.

There are two options of how to use this library:
 1) in a transparent mode where the library is simply hooked in and executes as a constructor
 2) in a cooperative mode where library functions are called to manually initialize and reset the FPU and SSE registers to desired values before and after enclave calls respectively.

While 1. is a convenient approach to change the FPU registers of applications without needing to touch their source code, 2. was e.g. used for the results in Table 1 and 3 to ensure that the untrusted application was not impacting itself with non-standard FPU registers.

In the cooperative mode, the library provides these core functions:
| Function                 | Description                   |
| :----------------------- |:------------------------------|
| faulty_point_init        | Initializes the FPU and SSE registers to desired values (controlled with environment variables as described below) |
| faulty_point_reset       | Resets the FPU and SSE registers to their value before initializtion was called. This prevents undesired behavior in the attacker controlled code. |
| **Convenience functions:**   | |
| fauly_points_get_mxcsr   | Returns the MXCSR value as uint32_t |
| fauly_points_set_mxcsr   | Sets the MXCSR                      |
| fauly_points_print_fpu_control_word | Provide a human readable printout of the FCW |
| fauly_points_get_fcw     | Returns the FCW as uint16_t         |
| fauly_points_set_fcw     | Sets the FCW                        |
| fauly_points_print_mxcsr | Provide a human readable printout of the MXCSR |
| fauly_points_dump_hex    | Convenience function to dump a buffer in hex. This can be useful to reduce undesired behavior by e.g. the printf function as it might round the output itself.|

You can control the library to set the FPU to desired values by modifying the following environment variables:
| Variable name   | Allowed values                | Description              |
| :-------------- | :---------------------------- | :----------------------- |
| FPU_ROUND       | UP DOWN TO_NEAREST or TO_ZERO | Sets the x87 and SSE rounding modes.|
| FPU_PRECISION   | SINGLE DOUBLE or EXTENDED     | Sets the x87 precision mode (does not exist for SSE)|
| FPU_MMX         | Only active when set to TRUE  | Marks the FPU tag word as full which prevents the program from using the FPU.|
| FPU_VERBOSE     | Only active when set to TRUE  | Enters a verbose mode that prints the MXCSR and FCW before and after execution and at every change by the library.|
| FPU_LIB_ENABLED | Only active when set to TRUE  | If set, uses a constructor and destructor to run before and after the C program to set the FPU mode before the program even executes. This is convenient for programs that are unaware of the library and just have it included at compile time. You do NOT need this variable for e.g. this project as it calls the faulty_points_init and faulty_points_reset functions manually. |

Examples for the manual usage can be deduced from the eval scripts in the Table 1 POC code, but here are some simple use cases:

Execution of a program with default parameters (should not differ from a call of the program without any environment variables set): `FPU_PRECISION=EXTENDED FPU_ROUND=TO_NEAREST ./prog`<br>

One of the modes with the largest floating point error is using single precision and rounding down: `FPU_PRECISION=SINGLE FPU_ROUND=DOWN ./prog`

Performing the MMX attack on double precision and default rounding mode (to nearest). Note, that the rounding or precision mode settings do not have any impact on the result of the MMX attack as it merely prevents utilization of the FPU due to all MMX registers being marked as in-use: `FPU_PRECISION=DOUBLE FPU_ROUND=TO_NEAREST FPU_MMX=TRUE ./inc`

All three of the above examples assume that the attacked program is calling the `faulty_point_init` function at least once to initialize the attack (e.g., this is usually done right before an ecall). However, if this is not the case, then the library can also be used to perform this initialization in a C constructor before execution of the program: `FPU_PRECISION=DOUBLE FPU_ROUND=TO_NEAREST FPU_MMX=TRUE FPU_LIB_ENABLED=TRUE ./inc`

To see a nice printout of this initialization phase, one can use `FPU_VERBOSE=TRUE` as in: `FPU_PRECISION=DOUBLE FPU_ROUND=TO_NEAREST FPU_MMX=TRUE FPU_LIB_ENABLED=TRUE FPU_VERBOSE=TRUE ./inc`