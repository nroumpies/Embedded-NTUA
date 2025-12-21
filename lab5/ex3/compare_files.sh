#!/bin/bash


# Check that exactly 2 arguments are provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 file1 file2"
    exit 1
fi

file1=$1
file2=$2

# Compare the files
cmp -s "$file1" "$file2"

if [ $? -eq 0 ]; then
    echo "Files are identical"
else
    echo "Files are different"
fi
