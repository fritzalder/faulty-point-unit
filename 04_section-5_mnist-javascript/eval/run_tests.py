#!/usr/bin/env python3
# Contains some unnecessary imports
import sys
import argparse
import json, base64
import socket
import os
import subprocess
import csv
import datetime
import time
import sys
import itertools
from tqdm import tqdm
import json
# Logging
import logging
import logging.config
logging.config.fileConfig('logging.conf')
logger = logging.getLogger('client')



def init_logging(verbose = True):
    """
    Create and set up Logger
    """
    loglevel = (logging.DEBUG if verbose else logging.INFO)
    logger.setLevel(loglevel)
    logger.info("EVAL RUNNER for SGX+FaaS")


def parse_args():
    parser = argparse.ArgumentParser(description="SGX Floats fuzzing")
    parser.add_argument(
        "-a", "--amount",
        type=int, required=False, default=100,
        help="The number of runs per input file.",
    )
    parser.add_argument(
        "-i", "--initial",
        type=int, required=False, default=0,
        help="The start index where the run should start at.",
    )
    parser.add_argument(
        "-x", "--executable",
        type=str, required=False, default="../sgx-duktape/app",
        help="The Duktape executable (can be SGX).",
    )
    parser.add_argument(
        "-e", "--enclave",
        type=str, required=False, default="../sgx-duktape/enclave.signed.so",
        help="If using SGX, the path to the enclave .so file.",
    )
    parser.add_argument(
        "-p", "--script-path", default = '../eval',
        type=str, required=False,
        help="The path to this script file, relative from the executable directory.",
    )
    parser.add_argument(
        "-j", "--javascript",
        type=str, required=False, default="../js-files/convnet.js",
        help="Javascript file to run.",
    )
    parser.add_argument(
        "-n", "--neural-net",
        type=str, required=False, default="../js-files/trained-network.json",
        help="Neural net to run.",
    )
    parser.add_argument(
        "-m", "--mnist",
        type=str, required=False, default="../js-files/mnist_handwritten_test_first100.json",
        help="MNIST test input to evalute.",
    )
    parser.add_argument(
        "-o", "--out", default = '.',
        type=str, required=False,
        help="The base path for the evaluation results relative to this python file.",
    )
    parser.add_argument(
        "-v", "--verbose",
        required=False, default=False, action='store_true',
        help="Log verbosely.",
    )
    parser.add_argument(
        "--ci",
        required=False, default=False, action='store_true',
        help="Use a fixed deterministic directory name to ease CI scripting.",
    )
    parser.add_argument(
        "-s", "--simulator",
        required=False, default=False, action='store_true',
        help="Use simulator instead of hw mode.",
    )
    parser.add_argument(
        "-f", "--prefix",
        type=str, required=False, default="",
        help="Prefix for output filenames.",
    )
    return parser.parse_args()

def create_dir_if_not_exist(dirname):
    if not os.path.isdir(dirname):
        os.mkdir(dirname)
        logger.info("Created directory " + dirname)

def filter_debug(s):
    return s[s.find('{'):]

def main():
    args = parse_args()
    init_logging(args.verbose)

    out_base_path = os.path.normpath(args.out)
    create_dir_if_not_exist(out_base_path)
    
    # Get path of Duktape executable
    executable_path = os.path.dirname(args.executable)

    # Determine whether we want simulation or hardware mode
    run_in_hw = True
    if args.simulator:
        run_in_hw = False

    compile_x87_script = "compile_fpu_hwmode.sh" if run_in_hw else "compile_fpu_simulator.sh"
    compile_sse_script = "compile_sse_hwmode.sh" if run_in_hw else "compile_sse_simulator.sh"
    
    # Get basic info
    timestamp = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d_%H-%M-%S')
    start_index = args.initial
    stop_index = args.amount + start_index
    filename = timestamp + '_' + str(start_index) + '-' + str(stop_index) 
    filename += "_HW-RUN" if run_in_hw else "_SIM-RUN"
    # Reset filename to a fixed string if we use ci.
    if args.ci:
        filename = "test_output"
    out_path = os.path.normpath(out_base_path + '/' + filename )
    create_dir_if_not_exist(out_path)
    logger.info("Using " + out_path + " as base directory for outputs")

    # Create individual script input
    with open(args.neural_net) as nn_fp:
        neural_net = json.load(nn_fp)
    with open(args.mnist) as mnist_fp:
        mnist = json.load(mnist_fp)
    script_input = {"network":neural_net["network"], "input":mnist, "start_index": start_index, "end_index": stop_index}

    input_file = out_path + "/" + "input.json"
    with open(input_file, 'w') as outfile:
        json.dump(script_input, outfile)


    # Method to run one iteration of the tests. We always run twice: Once in SSE and once in FPU mode
    def run_test_round(file_prefix=""):
        # Run script on this input
        script_filename = args.javascript
        if not os.path.isfile(script_filename):
            logger.info("Script %s is not a file. Exiting.", script_filename)
            sys.exit(-1)

        fpu_round_list = ['UP', 'DOWN', 'TO_NEAREST', 'TO_ZERO']
        fpu_precision_list = ['SINGLE', 'EXTENDED']
        combined_list = list(itertools.product(fpu_precision_list,fpu_round_list))
        logger.info("Executing for inputs: " + str(combined_list))
        processes = []
        logger.info("Starting subprocesses..")
        for it in tqdm(combined_list):
            # Set env var
            curr_env = os.environ.copy()
            curr_env["FPU_PRECISION"] = it[0]
            curr_env["FPU_ROUND"] = it[1]

            arguments = [args.executable, \
                    '--execute=' + script_filename, \
                    '--input=' + os.path.join(args.script_path, input_file), \
                    '--out_file=' + os.path.join(args.script_path, out_path, args.prefix + file_prefix + it[0]+'_'+it[1]+'.log')
                    #'-d', \
                    #'-v', \
                    #'-e'
                    ]

            if args.enclave != "":
                # add enclave path to arguments
                arguments.append('--enclave=' + args.enclave)

            # Run the process
            processes.append(subprocess.Popen(arguments, stdout=subprocess.PIPE, env=curr_env))
                
        logger.info("done. All processes started.")
        logger.info("Waiting for completion...")
        for it in tqdm(range(len(combined_list))):
            processes[it].wait()
            logger.debug(processes[it].stdout.read())


    logger.info("Compiling enclave first in x87 mode..")
    # Compile enclave once in x87 mode
    subprocess.run(executable_path + "/" + compile_x87_script, shell=True, check=True)
    logger.info("Starting first round of tests")
    # Run first round of tests
    run_test_round(file_prefix="")
    
    logger.info("Second step: Compiling enclave in SSE mode..")
    # Compile enclave once in x87 mode
    subprocess.run(executable_path + "/" + compile_sse_script, shell=True, check=True)
    logger.info("Starting second round of tests")
    # Run first round of tests
    run_test_round(file_prefix="sse_")
    
    logger.info("Executed all input settings. Done.")

if __name__ == '__main__':
    main()

