#!/bin/bash

# A simple test runner for the Super language

TEST_DIR="tests"
INTERPRETER="python super.py"
PASSED_COUNT=0
FAILED_COUNT=0

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Ensure the script can be run from any directory
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

echo "Running Super language tests..."
echo "============================="

for test_file in ${TEST_DIR}/*.super; do
    base_name=$(basename "${test_file}" .super)
    expected_file="${TEST_DIR}/${base_name}.expected"

    if [ ! -f "${expected_file}" ]; then
        echo -e "🤔 Skipping ${base_name}: No .expected file found."
        continue
    fi

    echo -n "Running test: ${base_name}... "

    # Run the interpreter and capture the output
    output=$($INTERPRETER "${test_file}" 2>&1)

    # Get the expected output
    expected_output=$(cat "${expected_file}")

    # Normalize outputs by removing trailing whitespace from each line and at the end of the file
    output_normalized=$(echo -n "$output" | sed 's/[[:space:]]*$//')
    expected_output_normalized=$(echo -n "$expected_output" | sed 's/[[:space:]]*$//')


    if [ "$output_normalized" == "$expected_output_normalized" ]; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED_COUNT++))
    else
        echo -e "${RED}FAILED${NC}"
        ((FAILED_COUNT++))
        echo "-----[ ACTUAL OUTPUT ]-----"
        echo "$output"
        echo "-----[ EXPECTED OUTPUT ]----"
        echo "$expected_output"
        echo "--------------------------"
    fi
done

echo "============================="
echo "Test summary: ${PASSED_COUNT} passed, ${FAILED_COUNT} failed."
echo

if [ ${FAILED_COUNT} -ne 0 ]; then
    exit 1
fi

exit 0
