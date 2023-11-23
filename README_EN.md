# ESP32-C3-DevKitM-1 Sesame5 Switch Control Example

This project demonstrates how to use the ESP32-C3-DevKitM-1 microcontroller to register and control a Sesame5 smart lock. Utilizing the ESP-IDF development framework and BLE technology, this example automatically searches for, connects to, and registers nearby Sesame5 devices. When the ESP32-C3-DevKitM-1 detects that the Sesame5 is in the unlock position, it issues a command to automatically lock it.

## Other Languages
- [返回中文版](README.md)
- [日本語版](README_JP.md)

## Prerequisites
You will need to install ESP-IDF, which can be done using the `install.sh` script from the ESP-IDF to install necessary toolchains and dependencies.

## Installation and Environment Setup
1. Ensure that you have installed the toolchain through ESP-IDF's `install.sh`.
2. Open a terminal, navigate to the ESP-IDF path, and run `export.sh` to add it to your environment variables.
3. Connect your ESP32-C3-DevKitM-1 to your computer via USB.
4. Return to the project folder and run `idf.py flash` to compile and flash.

## Usage
After flashing and rebooting the ESP32-C3-DevKitM-1, it will automatically search for unregistered Sesame devices nearby. Once connected and registered, the ESP32-C3-DevKitM-1 will monitor the status of the Sesame5 and issue a lock command at the appropriate time.

## Features and Functionality
- **Automatic Device Discovery**: Automatically searches for and connects to nearby Sesame5 smart locks.
- **Automatic Locking**: Issues a lock command when the Sesame5 reaches the preset unlock position.

## Source Reference
This example is modified from the nimble BLE Central Example within ESP-IDF.

## Additional Resources
- Visit the [CANDY HOUSE Official Website](https://jp.candyhouse.co/) for more information about the Sesame5 smart lock.

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments
Thank you for your interest in this project and the support of open resources by CANDY HOUSE.
