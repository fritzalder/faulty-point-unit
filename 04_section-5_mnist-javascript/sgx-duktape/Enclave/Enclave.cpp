#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */
#include "duktape.h"
#include "string.h"
duk_context * ctx;
bool verbose_mode = false;

const char* script_output;

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

#define LOG(s){ printf("[ENCLAVE] %s", s); }
#define DEBUG(s){ if(verbose_mode){ LOG(s); } }

void ecall_script_init(const char* script, size_t script_len){
    ctx = duk_create_heap_default();
    if (!ctx) { printf("ERROR\n"); return; }
    duk_compile_string(ctx, DUK_COMPILE_FUNCTION, script);
}

size_t ecall_script_run(
    const char* input, 
    size_t input_size)
{
    // push the input
    duk_push_string(ctx, input);
    DEBUG("Input pushed to Duktape stack\n");
    
    duk_json_decode(ctx, -1);
    DEBUG("Input decoded by Duktape\n");

    // execute function
    duk_call(ctx, 1);
    DEBUG("Script executed on input.\n");

    script_output = duk_get_string(ctx, -1);

    printf("[ENCLAVE] Out: %s\n", script_output);

    return strlen(script_output);
}

void ecall_get_script_output(char* output_pointer, size_t output_pointer_size){
    size_t len = strlen(script_output);
    if(output_pointer_size < len){
        printf("ERROR: Size mismatch");
        return;
    }

    strncpy(output_pointer, script_output, len);
}

void ecall_verbose_mode(){
    verbose_mode = true;
    LOG("Enabled verbose mode\n");
}
