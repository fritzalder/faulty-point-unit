#!/usr/bin/python3

import argparse
from ctypes import *
import random
from subprocess import Popen, PIPE

def parse_args():
  parser = argparse.ArgumentParser(description="SGX Floats Evaluation script")
  parser.add_argument(
      "-a", "--amount",
      type=int, required=False, default=1000,
      help="Amounto of tests to be run. Paper data reported 1000 runs (default).",
  )

  return parser.parse_args()

  

TARGET = "./main"
MAX_SUBNORMAL_DOUBLE = 0x000FFFFFFFFFFFF
MIN_NORMAL_POSITIVE_DOUBLE = 0x0010000000000000

EXIT_NO_EXCEPTION = 0
EXIT_EXCEPTION = 1
EXIT_ERROR = 255

# how close do low and high have to be to stop
ERROR_EPSILON = .00001e-308

# Inspired by https://stackoverflow.com/a/1592200
def hex_to_double(s):
    i = int(s, 16)                   # convert from hex to a Python int
    cp = pointer(c_long(i))           # make this into a c integer
    fp = cast(cp, POINTER(c_double))  # cast the int pointer to a float pointer
    return fp.contents.value         # dereference the pointer, get the float
	
def double_to_hex(d):
    fp = pointer(c_double(d))           # make this into a c double
    cp = cast(fp, POINTER(c_long))  # cast the double pointer to a long pointer
    i = cp.contents.value         # dereference the pointer, get the int
    return hex(i)

def test_conversion():
    print(hex_to_double("3fe4d4fdf3b645a2")) # should print 0.651
    print(double_to_hex(0.651)) # should print 3fe4d4fdf3b645a2

def run_target(secret, input):
    secret_hex = double_to_hex(secret)
    input_hex = double_to_hex(input)
    process = Popen([TARGET, secret_hex, input_hex], stdout=PIPE)
    (output, err) = process.communicate()
    exit_code = process.wait()
    return (output, exit_code)

MIN_NORMAL_POSITIVE_DOUBLE_VAL = hex_to_double(hex(0x0010000000000000))

def recover_secret_binary_search(secret, debug = False):
    # Test if secret >= 1
    (output, result) = run_target(secret, hex_to_double(hex(MIN_NORMAL_POSITIVE_DOUBLE)))
    
    if result == EXIT_ERROR:
        print("Error returned, aborting...")
        return (0, -1)
    elif result == EXIT_NO_EXCEPTION:
        print("|value| >= 1 - aborting...")
        return (0, 1)
    else:
        print("|value| < 1, continue...")
    
    # Now comes the actual binary search
    low = 0.0
    high = 1.0
    cnt = 0

    while abs(low - high) > ERROR_EPSILON:
        mid = (high + low) / 2.0
        
        recovered_val = MIN_NORMAL_POSITIVE_DOUBLE_VAL/mid

        if debug == True:
            print("Run " + str(cnt) + ": H = " + str(high) + ", L = " + str(low) + ", M = " +  str(mid) + ", recovered = " + str(recovered_val))

        (output, result) = run_target(secret, mid)
        cnt = cnt + 1

        # abort on error
        if result == EXIT_ERROR:
            print("Error returned, aborting...")
            return (cnt, -1)
        # if we got an exception, we need to search in the upper half
        elif result == EXIT_EXCEPTION:
            low = mid
        # Else search in the lower half
        elif result == EXIT_NO_EXCEPTION:
            high = mid
    
    return (cnt, MIN_NORMAL_POSITIVE_DOUBLE_VAL/mid)


def main():
    args = parse_args()

    # Sample fixed test cases for bugfixing
    #test_cases = [1.2, 0.623, 0.001, 0.987654321, 0.123456, 0.5000001, 0.4999999]

    # Number of random tests
    NUM_TESTS = args.amount
    print("Performing %s tests." % str(NUM_TESTS))

    # Fixed seed for reproducibility
    random.seed(0)

    # Get test cases
    test_cases = [random.uniform(0, 1) for _ in range(NUM_TESTS)]

    OUT_FILE = "results.csv"
    print("Output file is " + OUT_FILE)
    f = open(OUT_FILE, "w")
    f.write("secret;recovered;error;steps\n")
    f.flush()

    for secret in test_cases:
        print()
        print("========================================================================")
        print()
        print("Searching for secret = " + str(secret))
        (cnt, recovered) = recover_secret_binary_search(secret)
        error = abs(secret - recovered)
        print("Recovered = " + str(recovered) + " after " + str(cnt) + " invocations")
        print("Error = " + str(error))

        # Output to CSV
        f.write(str(secret) + ";" + str(recovered) + ";" + str(error) + ";" + str(cnt) + "\n")
        f.flush()

    f.close()

    print("Performed %s tests." % str(NUM_TESTS))
    print("Output file is " + OUT_FILE)


if __name__ == '__main__':
    main()

