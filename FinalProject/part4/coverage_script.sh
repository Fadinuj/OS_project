#!/bin/bash

echo "=== Coverage Analysis for Random Euler Graph Generator ==="
echo "Current directory: $(pwd)"
echo "Moving to parent directory to access part2 and part3..."
cd ..
echo "Directory structure:"
ls -la
echo ""

echo "Creating part4/coverage directory..."
mkdir -p part4/coverage

echo "Cleaning old coverage files and binaries..."
rm -f part3/*.gcda part3/*.gcno part3/*.gcov part3/random part3/test_coverage part3/test_coverage.c
rm -f part4/coverage/*.gcov

echo ""
echo "Checking if files exist..."
if [ ! -d "part3" ]; then
    echo "ERROR: part3 directory not found!"
    echo "Available directories:"
    ls -d */ 2>/dev/null || echo "No directories found"
    exit 1
fi

if [ ! -f "part3/random.c" ]; then
    echo "ERROR: part3/random.c not found!"
    echo "Files in part3:"
    ls -la part3/
    exit 1
fi

if [ ! -f "part2/graph.c" ]; then
    echo "ERROR: part2/graph.c not found!"
    echo "Files in part2:"
    ls -la part2/ 2>/dev/null || echo "part2 directory not found"
    exit 1
fi

echo "All required files found!"
cd part3

echo "Compiling random.c with coverage flags..."
gcc -o random random.c ../part2/graph.c -fprofile-arcs -ftest-coverage -I../part2

echo ""
echo "=== Running basic test cases ==="

# Test 1: Basic successful cases
echo "Test 1: Small triangle (should have Euler circuit)"
./random -v 3 -e 3 -r 42

echo ""
echo "Test 2: Square graph"
./random -v 4 -e 4 -r 123

echo ""
echo "Test 3: Larger graph"
./random -v 6 -e 8 -r 456

echo ""
echo "Test 4: Complete graph K4 (should have Euler circuit)"
./random -v 4 -e 6 -r 789

echo ""
echo "Test 5: Small graph with few edges"
./random -v 5 -e 2 -r 111

echo ""
echo "Test 6: Chain graph (no Euler circuit)"
./random -v 6 -e 5 -r 222

echo ""
echo "=== Testing error conditions ==="

# Test error conditions
echo "Test 7: Invalid number of vertices (0)"
./random -v 0 -e 5 -r 1 || echo "Expected failure"

echo ""
echo "Test 8: Negative vertices"
./random -v -3 -e 5 -r 1 || echo "Expected failure"

echo ""
echo "Test 9: Negative edges"
./random -v 5 -e -2 -r 1 || echo "Expected failure"

echo ""
echo "Test 10: Too many edges for graph size"
./random -v 3 -e 10 -r 1 || echo "Expected failure"

echo ""
echo "Test 11: Missing parameters"
./random -v 5 || echo "Expected failure"

echo ""
echo "Test 12: Wrong number of arguments"
./random -v 5 -e 3 || echo "Expected failure"

echo ""
echo "Test 13: Invalid option"
./random -v 5 -e 3 -x 1 || echo "Expected failure"

echo ""
echo "=== Edge cases ==="

echo "Test 14: Single vertex, no edges"
./random -v 1 -e 0 -r 1

echo ""
echo "Test 15: Two vertices, one edge"
./random -v 2 -e 1 -r 1

echo ""
echo "Test 16: Self-loops test"
./random -v 2 -e 2 -r 333

echo ""
echo "Test 17: Maximum edges for small graph"
./random -v 3 -e 6 -r 444

# טסט memory stress (אם אפשר)
ulimit -v 50000  # הגבל זיכרון
./random -v 50 -e 100 -r 123

echo ""
echo "=== Searching for Euler circuit examples ==="
echo "Testing multiple seeds to find Euler circuits..."

# Try to find graphs with Euler circuits
for seed in 1 2 3 5 7 10 13 17 20 25 30 42 50 77 99 100 123 150 200 250 300 333 400 456 500 777 888 999
do
    echo "Trying seed $seed for 4 vertices, 4 edges..."
    if ./random -v 4 -e 4 -r $seed | grep -q "Euler circuit exists"; then
        echo "*** FOUND EULER CIRCUIT with seed $seed! ***"
        break
    fi
done

echo ""
echo "=== Generating coverage reports ==="
echo "Moving to generate coverage reports and save in part4/coverage..."

# Generate coverage reports and move them to part4/coverage
gcov random-random.c
gcov random-graph.c

# Move the generated gcov files to part4/coverage directory
mv *.gcov ../part4/coverage/ 2>/dev/null || echo "No .gcov files to move"

# Remove compiled executables
rm -f random

# Remove coverage data files
rm -f *.gcda *.gcno

# Check what was created in part4/coverage

echo ""
echo "Coverage analysis complete!"
