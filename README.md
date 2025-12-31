# ESP32-C3 Bare Metal Program

A bare metal LED blink program for ESP32-C3 using ESP-IDF framework in VSCode.

## Features

- Direct GPIO control for LED blinking
- Configured for ESP32-C3 microcontroller
- VSCode ESP-IDF extension support
- Minimal bare metal implementation

## Hardware Requirements

- ESP32-C3 development board
- USB cable for programming and power
- LED connected to GPIO8 (or modify `CONFIG_BLINK_GPIO` in sdkconfig.defaults)

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
│   ├── main.c              # Main application code
│   └── CMakeLists.txt      # Component build configuration
├── .vscode/
│   ├── settings.json       # VSCode ESP-IDF settings
│   ├── c_cpp_properties.json
│   └── launch.json         # Debug configuration
├── CMakeLists.txt          # Root project configuration
├── sdkconfig.defaults      # Default ESP32-C3 configuration
├── Kconfig.projbuild       # Project Kconfig options
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

You can modify the GPIO pin used for the LED in two ways:

1. **Edit sdkconfig.defaults:**
   ```
   CONFIG_BLINK_GPIO=8
   ```

2. **Using menuconfig:**
   ```bash
   idf.py menuconfig
   ```
   Navigate to `ESP32-C3 Bare Metal Configuration` → `Blink GPIO number`

## Code Explanation

The program demonstrates bare metal programming concepts:

- **GPIO Configuration**: Direct hardware initialization using ESP-IDF drivers
- **FreeRTOS Tasks**: Uses FreeRTOS for task scheduling
- **Logging**: ESP_LOGI for debug output
- **Timing**: vTaskDelay for precise timing control

## Serial Monitor Output

Expected output:
```
I (xxx) bare-metal: ESP32-C3 Bare Metal Program Starting...
I (xxx) bare-metal: GPIO 8 configured as output
I (xxx) bare-metal: LED OFF
I (xxx) bare-metal: LED ON
I (xxx) bare-metal: LED OFF
...
```

## Troubleshooting

1. **Build fails**: Ensure ESP-IDF is properly installed and environment is sourced
2. **Flash fails**: Check USB connection and ensure correct port is selected
3. **No LED blink**: Verify GPIO pin matches your hardware setup
4. **Permission denied**: On Linux, add user to dialout group: `sudo usermod -a -G dialout $USER`

## License

This is free and unencumbered software released into the public domain.

## Resources

- [ESP32-C3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
- [ESP-IDF API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/)
