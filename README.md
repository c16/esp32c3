# ESP32-C3 TRUE Bare Metal Program

[![ESP32-C3 Build](https://github.com/YOUR_USERNAME/esp32c3/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/esp32c3/actions/workflows/build.yml)
[![Code Quality](https://github.com/YOUR_USERNAME/esp32c3/actions/workflows/code-quality.yml/badge.svg)](https://github.com/YOUR_USERNAME/esp32c3/actions/workflows/code-quality.yml)

A completely bare metal LED blink program for ESP32-C3 using direct register access without FreeRTOS.

## Features

- **TRUE Bare Metal** - No FreeRTOS task APIs used
- **Direct Register Access** - GPIO manipulation via hardware registers
- **Custom Delay Functions** - CPU cycle counting instead of OS delays
- **Flash Memory Testing** - Comprehensive test of 16MB flash with write/read/verify
- **LED Status Patterns** - Visual feedback for test results
- **Custom Partition Table** - 15MB storage partition for testing
- **Code Quality Tooling** - Clang-tidy integration for static analysis
- **CI/CD Pipeline** - Automated builds and code quality checks via GitHub Actions
- **Hardware-Level Control** - Direct memory-mapped I/O
- VSCode ESP-IDF extension support for building

## Hardware Requirements

- ESP32-C3 development board
- USB cable for programming and power
- LED connected to GPIO8 (modify `LED_GPIO` in main/main.c to change)

## Software Prerequisites

1. **ESP-IDF** (v4.4 or later)
   - Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/)

2. **VSCode** with ESP-IDF Extension
   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install the [ESP-IDF VSCode Extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)

## Project Structure

```
esp32c3-bare-metal/
├── main/
│   ├── main.c              # Bare metal code with flash testing
│   └── CMakeLists.txt      # Component build configuration
├── .github/
│   └── workflows/
│       ├── build.yml       # CI/CD: Build firmware
│       └── code-quality.yml # CI/CD: Code quality checks
├── .vscode/
│   ├── settings.json       # VSCode ESP-IDF settings
│   ├── c_cpp_properties.json
│   ├── launch.json         # Debug configuration
│   └── tasks.json          # Build and clang-tidy tasks
├── CMakeLists.txt          # Root project configuration
├── sdkconfig.defaults      # ESP32-C3 configuration with 16MB flash
├── partitions.csv          # Custom partition table (1MB app + 15MB storage)
├── .clang-tidy             # Clang-tidy configuration
├── run-clang-tidy.sh       # Script to run clang-tidy
├── generate-compile-db.sh  # Generate compilation database
├── .gitignore
└── README.md
```

## Building and Flashing

### Method 1: Using VSCode ESP-IDF Extension

1. Open this project folder in VSCode
2. Press `F1` and select `ESP-IDF: Set Espressif device target` → Select `esp32c3`
3. Press `F1` and select `ESP-IDF: Build your project`
4. Connect your ESP32-C3 board via USB
5. Press `F1` and select `ESP-IDF: Select port to use` → Select your device port
6. Press `F1` and select `ESP-IDF: Flash your project`
7. Press `F1` and select `ESP-IDF: Monitor your device` to view output

### Method 2: Using Command Line

```bash
# Set up ESP-IDF environment (run once per terminal session)
. $HOME/esp/esp-idf/export.sh

# Set target to ESP32-C3
idf.py set-target esp32c3

# Build the project
idf.py build

# Flash to device (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Build, flash, and monitor in one command
idf.py -p /dev/ttyUSB0 flash monitor
```

## Configuration

### Changing the LED GPIO

Edit the `LED_GPIO` definition in `main/main.c`:

```c
#define LED_GPIO    8  // Change to your desired GPIO pin
```

Supported GPIO pins on ESP32-C3: 0-21 (avoid pins used for flash/USB)

### Disabling Flash Test

To disable the flash memory test and reduce code size, edit `main/main.c`:

```c
#define FLASH_TEST_ENABLED      0  // Change from 1 to 0
```

## Code Explanation

This program demonstrates **TRUE bare metal programming** without FreeRTOS APIs:

### Direct Register Access
```c
#define GPIO_ENABLE_REG         (0x60004020)  // GPIO output enable register
#define GPIO_OUT_REG            (0x60004004)  // GPIO output data register
#define GPIO_FUNC_OUT_SEL_CFG   (0x60004554) // GPIO function select base
```

### Key Functions

1. **gpio_init_output()** - Directly writes to hardware registers to configure GPIO as output
2. **gpio_set_high() / gpio_set_low()** - Direct register manipulation to control pin state
3. **delay_ms()** - CPU cycle-based timing using RISC-V cycle counter (`esp_rom_get_ccount()`)
4. **write_reg() / read_reg()** - Volatile pointer access to memory-mapped I/O

### No FreeRTOS Dependencies
- No `vTaskDelay()` - uses cycle counting
- No task creation - runs in `app_main()` infinite loop
- No semaphores, queues, or other RTOS primitives
- Minimal runtime overhead

### Flash Memory Testing

The program includes comprehensive flash memory testing:

1. **test_flash_block()** - Erases, writes, reads, and verifies flash blocks
2. **test_flash_memory()** - Tests all available flash outside the program area
3. **LED Blink Patterns** - Visual feedback during testing:
   - Quick double blink: Testing in progress
   - 5 fast blinks: Test passed
   - 3 slow blinks: Test failed

The flash test:
- Detects total flash size (16MB on your board)
- Finds storage partition or tests upper half of flash
- Tests up to 100 blocks (4KB each) with alternating 0xAA/0x55 pattern
- Reports results via serial output
- Runs once on startup, then continues with LED blink loop

## Partition Table

Custom partition table for 16MB flash (`partitions.csv`):

| Partition | Type | SubType | Offset | Size | Purpose |
|-----------|------|---------|--------|------|---------|
| nvs | data | nvs | 0x9000 | 16KB | Non-volatile storage |
| phy_init | data | phy | 0xd000 | 4KB | PHY init data |
| factory | app | factory | 0x10000 | 1MB | Application firmware |
| storage | data | spiffs | 0x110000 | 15MB | Storage (used for flash testing) |

The 15MB storage partition provides ample space for flash testing without affecting the program.

## Serial Monitor Output

With logging enabled, you'll see flash test results on startup:

```
I (xxx) flash-test: Starting flash memory test...
I (xxx) flash-test: This will test flash memory outside program area
I (xxx) flash-test: Flash size: 16777216 bytes (16.00 MB)
I (xxx) flash-test: Found storage partition at 0x110000, size: 15728640 bytes
I (xxx) flash-test: Tested 100 blocks, stopping test
I (xxx) flash-test: === Flash Test Results ===
I (xxx) flash-test: Total flash size: 16777216 bytes
I (xxx) flash-test: Tested data: 25600 bytes
I (xxx) flash-test: Blocks tested: 100
I (xxx) flash-test: Errors: 0
I (xxx) flash-test: Flash test PASSED!
```

After the test completes, LED blinks normally:
- **ON** for 1 second
- **OFF** for 1 second
- Repeats indefinitely

## Why ESP-IDF if it's Bare Metal?

While this is bare metal code (no FreeRTOS APIs used), we still use ESP-IDF for:
- **Build system**: CMake configuration and compilation
- **Bootloader**: Initial chip startup and flash loading
- **Header files**: Hardware register definitions (`soc/gpio_reg.h`)
- **ROM functions**: Low-level routines like `esp_rom_get_ccount()`
- **Flashing tools**: esptool.py for uploading firmware

The actual application code is completely bare metal with direct hardware access.

## Hardware Register Reference

ESP32-C3 GPIO Registers (from memory map at 0x60004000):
- **0x60004004**: GPIO_OUT_REG - Output data
- **0x60004020**: GPIO_ENABLE_REG - Output enable
- **0x60004554**: GPIO_FUNCx_OUT_SEL_CFG - Function selection

See [ESP32-C3 TRM Chapter 5](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf) for complete register documentation.

## Code Quality: Clang-Tidy

This project includes clang-tidy configuration for static code analysis. Clang-tidy helps catch bugs, enforce coding standards, and improve code quality.

### Prerequisites

Install clang-tidy:
```bash
# Debian/Ubuntu
sudo apt-get install clang-tidy

# macOS
brew install llvm
```

### Running Clang-Tidy

#### Method 1: Command Line

**Check for issues (no modifications):**
```bash
./run-clang-tidy.sh
```

**Auto-fix issues:**
```bash
./run-clang-tidy.sh --fix
```

**Generate compilation database manually:**
```bash
./generate-compile-db.sh
```

#### Method 2: VSCode Tasks

Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on macOS) and select:
- **Tasks: Run Task** → **Run Clang-Tidy (Check Only)** - Analyze without changes
- **Tasks: Run Task** → **Run Clang-Tidy (Auto-Fix)** - Analyze and apply fixes
- **Tasks: Run Task** → **Generate Compile Database** - Regenerate compilation database

#### Method 3: Direct clang-tidy Command

```bash
# After building the project
clang-tidy -p build/compile_commands.json main/main.c
```

### Clang-Tidy Configuration

The `.clang-tidy` file includes checks for:
- **Bug detection**: Potential bugs and logic errors
- **Performance**: Performance anti-patterns
- **Readability**: Code clarity and maintainability
- **Modernization**: Modern C practices
- **Portability**: Cross-platform compatibility

Disabled checks:
- Magic numbers (common in embedded register programming)
- Non-const globals (necessary for hardware access)
- Some readability checks that conflict with embedded conventions

### Customizing Checks

Edit `.clang-tidy` to enable/disable specific checks:

```yaml
Checks: >
  -*,
  bugprone-*,
  performance-*,
  readability-*
```

Add your custom check disables after existing ones:
```yaml
  -readability-your-check-name
```

## Continuous Integration (CI/CD)

This project includes GitHub Actions workflows for automated building and code quality checks.

### Workflows

#### 1. Build Workflow (`build.yml`)

Automatically builds the firmware on every push and pull request:

- **Triggers**: Push to main/master/develop/claude/\* branches, pull requests
- **Actions**:
  - Sets up ESP-IDF v5.1.2 environment
  - Builds the project with `idf.py build`
  - Shows binary size information
  - Uploads firmware artifacts (.bin, .elf, .map files)
  - Uploads compile_commands.json for analysis

**Artifacts** (available for 30 days):
- `esp32c3-firmware` - All compiled binaries
- `compile-commands` - Compilation database

#### 2. Code Quality Workflow (`code-quality.yml`)

Runs static analysis and formatting checks:

- **Triggers**: Push to main/master/develop/claude/\* branches, pull requests
- **Actions**:
  - Runs clang-tidy analysis
  - Checks file permissions
  - Checks for trailing whitespace
  - Generates code statistics

**Artifacts** (available for 30 days):
- `clang-tidy-results` - Full clang-tidy analysis output

### Viewing Workflow Results

1. Go to the **Actions** tab in your GitHub repository
2. Select a workflow run to see detailed logs
3. Download artifacts from the workflow run summary
4. Check the workflow summary for quick stats and issues

### Local vs CI Builds

The CI environment uses:
- ESP-IDF v5.1.2
- Ubuntu latest
- Standard ESP-IDF build tools

Ensure your local environment matches for consistent results.

### Badge Status

The badges at the top of this README show real-time build and code quality status:
- 🟢 Green: All checks passing
- 🔴 Red: Build failed or quality issues found
- 🟡 Yellow: Workflow running

**Note**: Update the badge URLs in README.md with your GitHub username/repository name:
```markdown
[![ESP32-C3 Build](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml)
```

## Troubleshooting

1. **Build fails**: Ensure ESP-IDF is properly installed and environment is sourced
2. **Flash fails**: Check USB connection and ensure correct port is selected
3. **No LED blink**: Verify GPIO pin matches your hardware setup
4. **Permission denied**: On Linux, add user to dialout group: `sudo usermod -a -G dialout $USER`
5. **LED blinks wrong speed**: Adjust CPU frequency in sdkconfig or modify delay_cycles() multiplier

## License

This is free and unencumbered software released into the public domain.

## Resources

- [ESP32-C3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
- [ESP-IDF API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/)
