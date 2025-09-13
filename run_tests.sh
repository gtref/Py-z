#!/bin/bash

# Exit on error
set -e

# Build the compiler
echo "Building the compiler..."
make

# Check if the compiler was built
if [ ! -f ./super ]; then
    echo "Compiler executable not found. Cannot run tests."
    exit 1
fi

echo "Running tests..."

for test_file in tests/*.super; do
    expected_file="${test_file%.super}.expected"
    test_name=$(basename "$test_file")

    echo -n "Testing $test_name... "

    # Run the compiler and capture the output
    actual_output=$(./super "$test_file")

    # Read the expected output
    expected_output=$(cat "$expected_file")

    # Compare the actual output with the expected output
    if [ "$actual_output" == "$expected_output" ]; then
        echo "PASS"
    else
        echo "FAIL"
        echo "  Expected: $expected_output"
        echo "  Actual:   $actual_output"
        exit 1
    fi
done

echo "All tests passed!"
