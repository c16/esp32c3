#!/bin/bash
# Script to run clang-tidy on ESP32-C3 bare metal project
# Usage: ./run-clang-tidy.sh [--fix]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}ESP32-C3 Clang-Tidy Runner${NC}"
echo "================================"

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    echo -e "${RED}Error: clang-tidy not found!${NC}"
    echo "Install it with: sudo apt-get install clang-tidy"
    exit 1
fi

# Check if compile_commands.json exists
if [ ! -f "build/compile_commands.json" ]; then
    echo -e "${YELLOW}compile_commands.json not found. Generating...${NC}"
    echo ""

    # Check if ESP-IDF is sourced
    if [ -z "$IDF_PATH" ]; then
        echo -e "${RED}Error: ESP-IDF environment not sourced!${NC}"
        echo "Run: source \$HOME/esp/esp-idf/export.sh"
        exit 1
    fi

    # Build the project to generate compile_commands.json
    echo "Building project to generate compilation database..."
    idf.py build
    echo ""
fi

# Determine if we should fix issues
FIX_FLAG=""
if [ "$1" == "--fix" ]; then
    FIX_FLAG="--fix"
    echo -e "${YELLOW}Running clang-tidy with automatic fixes...${NC}"
else
    echo -e "${YELLOW}Running clang-tidy (analysis only)...${NC}"
    echo "Tip: Use './run-clang-tidy.sh --fix' to apply fixes automatically"
fi
echo ""

# Run clang-tidy on main source files
echo "Analyzing main/main.c..."
clang-tidy \
    -p build/compile_commands.json \
    $FIX_FLAG \
    main/main.c \
    --config-file=.clang-tidy

echo ""
echo -e "${GREEN}Clang-tidy analysis complete!${NC}"

# Optional: Run on all C files in the project
# Uncomment the following to analyze all files:
#
# echo ""
# echo "Analyzing all C files..."
# find main -name "*.c" -exec clang-tidy -p build/compile_commands.json $FIX_FLAG {} \;
