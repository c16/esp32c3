# ESP32-C3 TRUE Bare Metal Program

A completely bare metal LED blink program for ESP32-C3 using direct register access without FreeRTOS.

## Features

- **TRUE Bare Metal** - No FreeRTOS task APIs used
- **Direct Register Access** - GPIO manipulation via hardware registers
- **Custom Delay Functions** - CPU cycle counting instead of OS delays
- **Minimal Code Size** - Optimized configuration with disabled logging
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
│   ├── main.c              # Bare metal code with direct register access
│   └── CMakeLists.txt      # Component build configuration
├── .vscode/
│   ├── settings.json       # VSCode ESP-IDF settings
│   ├── c_cpp_properties.json
│   └── launch.json         # Debug configuration
├── CMakeLists.txt          # Root project configuration
├── sdkconfig.defaults      # Minimal ESP32-C3 configuration
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

## Serial Monitor Output

Since logging is disabled for minimal code size, there is **no serial output**. The LED will simply blink on GPIO8:
- **ON** for 1 second
- **OFF** for 1 second
- Repeats indefinitely

To enable debug output, modify `sdkconfig.defaults`:
```
CONFIG_LOG_DEFAULT_LEVEL_INFO=y  # Change from NONE to INFO
```

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
