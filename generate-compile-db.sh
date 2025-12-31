#!/bin/bash
# Generate compile_commands.json for clang-tidy
# This is automatically done by idf.py build, but this script can be used standalone

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Generating compile_commands.json${NC}"
echo "===================================="

# Check if ESP-IDF is sourced
if [ -z "$IDF_PATH" ]; then
    echo -e "${RED}Error: ESP-IDF environment not sourced!${NC}"
    echo "Run: source \$HOME/esp/esp-idf/export.sh"
    exit 1
fi

# Set target to ESP32-C3 if not already set
if [ ! -f "sdkconfig" ]; then
    echo -e "${YELLOW}Setting target to esp32c3...${NC}"
    idf.py set-target esp32c3
fi

# Build the project (this generates compile_commands.json)
echo -e "${YELLOW}Building project to generate compilation database...${NC}"
idf.py build

# Check if compile_commands.json was created
if [ -f "build/compile_commands.json" ]; then
    echo ""
    echo -e "${GREEN}Success!${NC} compile_commands.json generated at:"
    echo "  $(pwd)/build/compile_commands.json"
    echo ""
    echo "File contains $(jq length build/compile_commands.json 2>/dev/null || echo "many") compilation entries"
    echo ""
    echo "You can now run clang-tidy with:"
    echo "  ./run-clang-tidy.sh"
else
    echo -e "${RED}Error: compile_commands.json not found after build!${NC}"
    exit 1
fi
