#!/bin/bash
# testing.sh - Test cases for the querier
# @author: Aniket Dey

chmod +x ../crawler/crawler
chmod +x ../indexer/indexer
chmod +x querier

# Clean up any previous test outputs
rm -f ../data/letters-1.index
rm -rf ../data/letters-1

# Directory and file paths
SEED_URL="http://cs50tse.cs.dartmouth.edu/tse/letters/index.html"
PAGE_DIR="../data/letters-1"
INDEX_FILE="../data/letters-1.index"
MAX_DEPTH=1

# 1. Run Crawler on /data/letters-1
echo "===== Running crawler on $SEED_URL with depth $MAX_DEPTH ====="
mkdir -p "$PAGE_DIR"
../crawler/crawler "$SEED_URL" "$PAGE_DIR" "$MAX_DEPTH"
echo ""

# 2. Run Indexer to Generate letters-1.index
echo "===== Running indexer on $PAGE_DIR to generate $INDEX_FILE ====="
../indexer/indexer "$PAGE_DIR" "$INDEX_FILE"
echo ""


# 3. Test Querier with Relevant Queries
echo "===== Testing querier with valid queries ====="
echo ""

# Function to run querier with a given query
run_querier_test() {
    test_name="$1"
    query="$2"

    echo "----- $test_name -----"
    echo "Query: $query"
    echo "$query" | ./querier "$PAGE_DIR" "$INDEX_FILE"
    echo ""
}

# Run valid query tests
run_querier_test "Test 1: Query 'playground'" "playground"

run_querier_test "Test 2: Query 'page'" "page"

run_querier_test "Test 3: Query 'home and tse'" "home and tse"

run_querier_test "Test 4: Query 'algorithm or tse'" "algorithm or tse"


# 4. Add Invalid Test Cases
echo "===== Testing querier with invalid queries ====="
echo ""

# Run invalid query tests
run_querier_test "Test 5: Invalid query starting with 'and'" "and playground"

run_querier_test "Test 6: Invalid query ending with 'or'" "page or"

run_querier_test "Test 7: Invalid query with adjacent 'and's" "playground and and page"

run_querier_test "Test 8: Invalid query with adjacent 'or's" "algorithm or or tse"

run_querier_test "Test 9: Invalid query with non-alphabetic characters" "playground @ page"

run_querier_test "Test 10: Empty query" ""

# 5. Test for Memory Leaks with Valgrind
echo "===== Testing querier for memory leaks with Valgrind ====="
echo ""

echo "Running Valgrind on 'playground' query"
echo "playground" | valgrind --leak-check=full --show-leak-kinds=all ./querier "$PAGE_DIR" "$INDEX_FILE"
echo ""

# 6. Clean Up
echo "Cleaning up data directories"
# rm -f ../data/letters-1.index
# rm -rf ../data/letters-1
echo "All tests completed."
