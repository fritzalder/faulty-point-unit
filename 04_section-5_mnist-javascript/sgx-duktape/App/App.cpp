#include "App.h"
extern "C" 
{
    #include "fpu_lib.h"
}
/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

struct arguments {
    char* progname;
    std::string execute;
    std::string execute_input;
    std::string out_file;
    std::string enclave_path;
    bool eval;
};

struct arguments args;
using namespace boost::program_options;
using namespace std;
sgx_status_t sgx_rv;
bool verbose_mode = false;

char* script_output;

#define LOG(s){ printf("[MAIN] %s\n", s); }
#define DEBUG_LOG(s){ if(verbose_mode){ LOG(s); } }

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


/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Step 1: try to retrieve the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave(const char* enclave_name)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;
    
    /* Step 1: try to retrieve the launch token saved by last transaction 
     *         if there is no token, then create a new one.
     */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;
    
    if (home_dir != NULL && 
        (strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
    } else {
        /* if token path is too long or $HOME is NULL */
        strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    }

    FILE *fp = fopen(token_path, "rb");
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }

    if (fp != NULL) {
        /* read the token from saved file */
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(enclave_name, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        if (fp != NULL) fclose(fp);
        return -1;
    }

    /* Step 3: save the launch token if it is updated */
    if (updated == FALSE || fp == NULL) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != NULL) fclose(fp);
        return 0;
    }

    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;
    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    fclose(fp);
    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

int parse_arguments(int argc, char* argv[], struct arguments *args, variables_map *vm)
{
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("execute,x", value<std::string>(&(args->execute)),
             "Execute a JS file")
            ("input,i", value<std::string>(&(args->execute_input)),
             "Input file to provide to JS")
             ("enclave,e", value<std::string>(&(args->enclave_path))->default_value(ENCLAVE_FILENAME),
             "Path to the enclave")
             ("verbose,v", "Enable logging mode")
             ("out_file,o", value<std::string>(&(args->out_file)), 
             "File to print JSON result to.");

        store(parse_command_line(argc, argv, desc), *vm);
        notify(*vm);
    } catch( const error &ex ) {
        std::cout << ex.what() << "\n";
        return -1;
    }
    return 0;
}

int load_file(const char* filename, std::string* result)
{
    ifstream input_file(filename, ios::in | ios::binary);
    if( input_file ) {
        input_file.seekg(0, ios::end);
        result->resize(input_file.tellg());
        input_file.seekg(0, ios::beg);
        input_file.read(&((*result)[0]), result->size());
        input_file.close();
    } else {
        printf("Could not open file %s.\n", filename);
        return -1;
    }
    return 0;
}

void load_and_run_script(std::string js_script, std::string json_input){
    DEBUG_LOG("Loading script...");
    SGX_ASSERT(ecall_script_init(global_eid, js_script.c_str(), js_script.length()+1));
    DEBUG_LOG("..done");

    DEBUG_LOG("Running script...");
    size_t output_size;
    SGX_ASSERT(ecall_script_run(global_eid, &output_size, json_input.c_str(), json_input.length() + 1));
    DEBUG_LOG("Running script done.");

    DEBUG_LOG("Receiving string size...");
    script_output = (char*)malloc(output_size+1);
    script_output[output_size] = '\0';
    SGX_ASSERT(ecall_get_script_output(global_eid, (char*)script_output, output_size));
    DEBUG_LOG("done.");
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    variables_map vm;
    sgx_status_t ret;
    std::string js_script = "";
    std::string json_input = "";
    

    if( parse_arguments(argc, argv, &args, &vm) != 0 )
        return -1;

    const char* execute_file = args.execute.c_str();
    const char* input_file = args.execute_input.c_str();
    const char* enclave_file = args.enclave_path.c_str();

    if(vm.count("verbose")){
        verbose_mode = true;
    }

    load_file(execute_file, &js_script);
    load_file(input_file, &json_input);
    // printf("JSON input is: %s\n", json_input.c_str());

    /* Initialize the enclave */
    if(initialize_enclave(enclave_file) < 0) {
        LOG("Init enclave failed");
        return -1; 
    }

    if(verbose_mode){
        SGX_ASSERT(ecall_verbose_mode(global_eid));
    }

    auto start = std::chrono::system_clock::now();
    load_and_run_script(js_script, json_input);
    auto end = std::chrono::system_clock::now();

    printf("\n");

    if(vm.count("out_file")){
        std::ofstream out(args.out_file);

        char timestamp[100];
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", std::localtime(&end_time));
        out << "Execution at: " << timestamp << '\n'
            << "elapsed time: " << elapsed_seconds.count() << "s\n";
        
        out << "Input script was: " << args.execute << '\n'
            << "Input to script was: " << args.execute_input << '\n'
            << "(Path of this file at time of creation for reference: " << args.out_file << " )" << '\n';
        
        // Set FPU attack again to nicely print FPU registers into output file
        faulty_point_init();
        fpu_control_t num;
        _FPU_GETCW(num);
        out << "FPU Control Word at time of execution:0x" << std::hex << num << std::dec << '\n';
        faulty_point_reset();
        
        out << "Output JSON:" << '\n';
        out << script_output << '\n';
        out.close();
    }

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    free((void*)script_output);

    return 0;
}

