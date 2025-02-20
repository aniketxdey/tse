#!/bin/bash
# testing.sh - Test cases for indexer and indextest files.
# @author: Aniket Dey

chmod +x indexer
chmod +x indextest

# Clean up any previous files
rm -f index.dat index2.dat

# Driver function to test indexer with valid crawler directory
test_valid_indexer() {
    dir=$1
    outfile=$2
    echo "Testing indexer with $dir"
    ./indexer $dir $outfile
    if [ -f $outfile ]; then
        echo "Successfully created $outfile"
        return 0
    else
        echo "Failed to create $outfile"
        return 1
    fi
}

#### 1. Invalid Test Cases
echo "Invalid test cases for indexer"
echo ""

# Test 1: Missing all parameters
echo "Test 1: Missing all parameters"
./indexer
echo ""

# Test 2: Missing one parameter
echo "Test 2: Missing one parameter"
./indexer ../data/letters-2
echo ""

# Test 3: Too many parameters
echo "Test 3: Too many parameters"
./indexer ../data/letters-2 index.dat extra
echo ""

# Test 4: Invalid pageDirectory (non-existent)
echo "Test 4: Invalid pageDirectory (non-existent path)"
./indexer /nonexistent/path index.dat
echo ""

# Test 5: Invalid pageDirectory (not crawler produced)
echo "Test 5: Invalid pageDirectory (not crawler directory)"
mkdir -p ../data/invalid-dir
./indexer ../data/invalid-dir index.dat
rm -rf ../data/invalid-dir
echo ""

# Test 6: Invalid indexFile (non-existent directory)
echo "Test 6: Invalid indexFile (non-existent directory path)"
./indexer ../data/letters-2 /nosuchdir/index.dat
echo ""

# Test 7: Invalid indexFile (read-only directory)
echo "Test 7: Invalid indexFile (read-only directory)"
mkdir -p ../data/readonly
chmod 555 ../data/readonly
./indexer ../data/letters-2 ../data/readonly/index.dat
rm -rf ../data/readonly
echo ""

#### 2. Valid Test Cases
echo "Valid test cases"
echo ""

# Test 8: Valid indexer test on letters-2
echo "Test 8: Valid indexer test on letters-2"
test_valid_indexer "../data/letters-2" "index.dat"
echo ""

# Test 9: Valid indextest
echo "Test 9: Valid indextest"
if [ -f index.dat ]; then
    ./indextest index.dat index2.dat
    echo ""
    
    # Compare files
    echo "Test 10: Comparing index files"
    if ~/cs50-dev/shared/tse/indexcmp index.dat index2.dat; then
        echo "Index files are identical"
    else
        echo "Index files differ!"
    fi
fi

#### 3. Memory Tests
echo "Memory leak testing"
echo ""

# Test 11: Valgrind on indexer
echo "Test 11: Testing indexer with valgrind"
valgrind --leak-check=full --show-leak-kinds=all ./indexer ../data/letters-2 index.dat
echo ""

# Test 12: Valgrind on indextest
echo "Test 12: Testing indextest with valgrind"
valgrind --leak-check=full --show-leak-kinds=all ./indextest index.dat index2.dat
echo ""

# Clean up
rm -f index.dat index2.dat
echo "All tests completed."