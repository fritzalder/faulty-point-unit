#!/usr/bin/env python3
import sys
import argparse
import json
import os
import subprocess
import datetime
import time
import itertools
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
    parser = argparse.ArgumentParser(description="SGX Floats Evaluation script")
    parser.add_argument(
        "-b", "--baseline",
        type=str, required=False, default="sse_EXTENDED_TO_NEAREST.log",
        help="File to use as baseline/reference anchor for the other files.",
    )
    parser.add_argument(
        "-p", "--path",
        type=str, required=True,
        help="The path for the evaluation results to be analyzed.",
    )
    parser.add_argument(
        "-f", "--file-header",
        type=int, required=False, default=7,
        help="Number of header lines to ignore in the evaluated files, i.e. lines before the JSON object.",
    )
    parser.add_argument(
        "-i", "--ignore-file",
        type=str, required=False, default=['input.json', 'Readme.md'], action='append',
        help="Filenames to ignore.",
    )
    parser.add_argument(
        "-v", "--verbose",
        required=False, default=False, action='store_true',
        help="Log verbosely.",
    )
    return parser.parse_args()

def create_dir_if_not_exist(dirname):
    if not os.path.isdir(dirname):
        os.mkdir(dirname)
        logger.info("Created directory " + dirname)

def load_file(filename, header_lines = 0):
    obj = {}
    with open(filename) as f:
        # Ignore potential header lines
        lines = f.readlines()
        if len(lines) >= header_lines:
        # Read JSON object and return it
            obj = json.loads(lines[header_lines].encode('utf-8').strip())
        else:
            logger.error("ERROR: File " + filename + " has only " +str(len(lines)) + " lines.")
            logger.error("Full file: " + str(lines))
            sys.exit(-1)
    return obj


def main():
    args = parse_args()
    init_logging(args.verbose)
 
    files_path = os.path.normpath(args.path)
    baseline_path = os.path.normpath(files_path + "/" + args.baseline)
    logger.info("Evaluating files in " + files_path)

    if not os.path.exists(files_path) or not os.path.isdir(files_path):
        logger.error("Input path does not exist. Exiting")
        sys.exit(-1)

    files = os.listdir(files_path)
    logger.info("Ignoring filenames " + str(args.ignore_file))
    for i in args.ignore_file:
        if i in files:
            files.remove(i)
    logger.info("Directory has files " + str(files))

    # Load baseline file
    if args.baseline not in files:
        logger.error("Baseline file " + args.baseline + " does not exist in directory " + files_path)
        sys.exit(-1)
    
    # baseline = load_file(baseline_path, header_lines=args.file_header)
    # baseline = baseline["results"] # Strip first nesting
    # files.remove(args.baseline)
    # logger.info("Loaded baseline file " + baseline_path)

    loaded_files = {}
    for it in files:
        loaded_files[it.split(".")[0]] = load_file(os.path.normpath(files_path + "/" + it), header_lines=args.file_header)["results"]
        # logger.debug("Loaded file " + it)

    # logger.debug("Loaded files:" + str(loaded_files))
    logger.info("Loaded all files.")
    baseline_key = args.baseline.split(".")[0]
    baseline = loaded_files[baseline_key]
    total_amount = len(baseline)
    logger.info("Baseline file " + args.baseline + " has " + str(total_amount) + " entries.")

    # Prepare statistics dict to keep track of all statistics
    dict_keys = loaded_files.keys()
    statistics = { k:{} for k in dict_keys }

    # Go through results
    for k in dict_keys:

        # Count general amount of correct guesses and amount of guesses that are equal to the baseline guess
        curr_correct_count = 0
        curr_as_baseline_count = 0

        # Count amount of predicted classes
        predicted_classes = [0 for i in range(10)]

        # Sum up the accumulated error for mean error
        summed_error = 0

        for i in range(len(loaded_files[k])):
            curr = loaded_files[k][i]

            # Correct guesses            
            if curr["correct"] is True:
                curr_correct_count += 1

            # Guesses as the baseline
            if curr["predicted"] == baseline[i]["predicted"]:
                curr_as_baseline_count += 1

            # Increase predicted class in list
            predicted_classes[curr["predicted"]] += 1

            for j in range(10): # 10 MNist classes per prediction set
                val = curr["predictions"][j]
                summed_error += abs(val - baseline[i]["predictions"][j])


        # Store correct guesses and the rate of correct guesses
        statistics[k]["correct_amount"] = curr_correct_count
        statistics[k]["correct_rate"] = curr_correct_count / total_amount

        # Store guesses that are equal to baseline and their rate
        statistics[k]["as_baseline_amount"] = curr_as_baseline_count
        statistics[k]["as_baseline_rate"] = curr_as_baseline_count / total_amount
        

        # Store predicted classes list
        statistics[k]["predicted_class_count"] = predicted_classes

        # Calculate average error
        statistics[k]["average_error"] = summed_error / (10 * total_amount)

    fpu_round_list = ['TO_NEAREST', 'DOWN', 'UP', 'TO_ZERO']
    fpu_precision_list = ['SINGLE', 'EXTENDED']
    
    single_zip = list(itertools.product(['SINGLE'],fpu_round_list))
    extended_zip = list(itertools.product(['EXTENDED'],fpu_round_list))
    single_precision = [ i[0] + "_" + i[1] for i in single_zip ]
    extended_precision = [ i[0] + "_" + i[1] for i in extended_zip ]
    sse_single = [ "sse_" + i[0] + "_" + i[1] for i in single_zip ]
    sse_extended = [ "sse_" + i[0] + "_" + i[1] for i in extended_zip ]
    
    rounding_modes = ["Round to nearest", "Rounding down", "Rounding up", "Round to zero"]
    print_format        = "{:<23} {:<17} {:<8} {:<7} {:<42} {:<42} {:01.50f}"
    print_format_header = "{:<23} {:<17} {:<8} {:<7} {:<42} {:<42} {:<50}"


    def pretty_print_dict(dict):
        print(print_format_header.format('CW','Rounding mode','Correct','(Rate)', 'Class count [0..9]', 'Class count difference to baseline [0..9]', 'Average error'))
        curr_rounding_mode = 0
        for k, v in sorted(dict.items(), key=lambda item: item[0]):
            class_count_string = "[%s]" % (" ".join(["{:<3}".format(s) for s in v["predicted_class_count"]]))
            incorrect_class_count_string = "[%s]" % (" ".join(["{:<3}".format(abs(s[0]-s[1])) for s in zip(v["predicted_class_count"],statistics[baseline_key]["predicted_class_count"])]))
            print(print_format.format(k, rounding_modes[curr_rounding_mode], v["as_baseline_amount"], v["as_baseline_rate"], class_count_string, incorrect_class_count_string, v["average_error"]))
            curr_rounding_mode += 1

    print("\nSingle precision:")
    pretty_print_dict({ k:v for k,v in statistics.items() if k in single_precision})

    print("\nExtended precision:")
    pretty_print_dict({ k:v for k,v in statistics.items() if k in extended_precision})

    print("\nSSE Single precision:")
    pretty_print_dict({ k:v for k,v in statistics.items() if k in sse_single})

    print("\nSSE Extended precision:")
    pretty_print_dict({ k:v for k,v in statistics.items() if k in sse_extended})

    for k,v in sorted(statistics.items(), key=lambda item: item[0]):
        logger.debug(k + " ".join(["" for s in range(6 - len(k))]) + ":" + str(v))


    

if __name__ == '__main__':
    main()

