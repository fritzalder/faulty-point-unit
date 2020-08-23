#!/bin/bash

# In our log files, the search string "Producing Raw Reports" is unique 
# and appears directly after the success and error messages.
# Keep in mind that this might not be the case for your log files or 
# for later versions of SPEC
SEARCH_STRING="Producing Raw Reports"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Printing Success/Error lists for each benchmark in log_files folder."

for f in $SCRIPT_DIR/log_files/*.log; do
    # Print file name
    echo "### Benchmark $(basename $f)"

    # Print two lines before the search string
    # But if the first line does not start with 'Success',
    # it could be that there are no errors, so we only
    # need one line before the search string
    OUTPUT=$(grep -B2 "$SEARCH_STRING" "$f" | sed '$d')
    
    if [[ $OUTPUT != Success* ]]; then
        # just print one line before the search string
        grep -B1 "$SEARCH_STRING" "$f" | sed '$d'
    else
        # Print two lines (Success: \n Error:)
        grep -B2 "$SEARCH_STRING" "$f" | sed '$d'
    fi

    echo " "

done


