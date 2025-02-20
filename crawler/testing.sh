#!/bin/bash
# testing.sh
# @author: Aniket Dey
# Comprehensive test cases for crawler


#### Invalid test cases
echo "Invalid test cases"
echo ""

# Test 1: Missing all parameters
echo "Test 1: Missing all parameters"
./crawler
echo ""
# Test 2: Missing one parameter (maxDepth)
echo "Test 2: Missing one parameter (maxDepth)"
./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html ../data/letters-missing-depth
echo ""

# Test 3: Too many parameters
echo "Test 3: Too many parameters"
./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html ../data/letters-too-many 10 extra_parameter
echo ""

# Test 4: Invalid seedURL (external URL)
echo "Test 4: Invalid seedURL (external URL)"
./crawler https://www.google.com ../data/invalid-external 2
echo ""

# Test 5: Invalid maxDepth (beyond allowed max depth)
echo "Test 5: Invalid maxDepth (beyond allowed max depth)"
./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html ../data/above-maxdepth 11
echo ""

#### Valgrind run
echo "Valgrind Testing"


# Test 6: Valgrind with 'toscrape' depth 1
echo "Test 6: Valgrind run on 'toscrape' at depth 1"
mkdir -p ../data/toscrape-1
valgrind --leak-check=full --show-leak-kinds=all ./crawler http://cs50tse.cs.dartmouth.edu/tse/toscrape/ ../data/toscrape-1 1
echo ""

echo "Valid Test Cases"

#### Valid Test Cases
# Test 7: 'letters' depth 1
echo "Test 7: 'letters' depth 1"
mkdir -p ../data/letters-1
./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html ../data/letters-1 1
echo ""

# Test 8: 'letters' depth 2
echo "Test 8: 'letters' depth 2"
mkdir -p ../data/letters-2
./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html ../data/letters-2 2
echo ""

# Test 9: 'toscrape' depth 0
echo "Test 9: 'toscrape' depth 1"
mkdir -p ../data/toscrape-0
./crawler http://cs50tse.cs.dartmouth.edu/tse/toscrape/ ../data/toscrape-0 0
echo ""

# Test 10: 'wikipedia' depth 0
echo "Test 10: 'wikipedia' depth 0"
mkdir -p ../data/wikipedia-0
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/ ../data/wikipedia-0 0
echo ""

# Test 11: 'wikipedia' depth 1
echo "Test 11: 'wikipedia' depth 1"
mkdir -p ../data/wikipedia-1
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/ ../data/wikipedia-1 1
echo ""

echo "All tests completed successfully."
echo ""
exit 0
