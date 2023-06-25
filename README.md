# Overlord
Code repository for the Overlord home automation system

This GitHub repository contains the code and documentation for a home automation project based on a custom board built around the ESP32-WROOM-32D processor. The board is designed to centralize home automation and provide control for various switches, low voltage devices, relay boards, power meters, and other modules. The primary goal of this project is to create a home automation system that is running as locally as possible while also providing an interface for existing home automation systems that can in turn control it in tandem.

## Features Overview
The custom board built on the ESP32 processor offers the following features:

- Digital Inputs: The board includes digital inputs that can be connected to various switches and sensors in your home. These inputs allow you to detect the state of switches and trigger automation actions based on their status.

- Digital Outputs: The board is equipped with digital outputs that can drive low voltage devices such as lights, fans, or other appliances. You can use these outputs to control the state of these devices based on your automation rules.

- Two MODBUS Lines: Two MODBUS lines are included on the board, enabling communication with relay boards and power meters. This allows you to monitor and control multiple devices and measure power consumption within your home automation system.

- CAN line: The board features a CAN (Controller Area Network) line that provides user input and control capabilities for other devices and modules. You can connect compatible devices to this line and communicate with them using the CAN protocol. This allows easy integration into any existing home automation system that supports CAN.

- Power output for other devices which can be chosen on the board and can be set to +12V, +5V, or +3.3V.

## Getting Started

To get started with this home automation project, follow these steps:

- Hardware Setup: Set up the custom board based on the ESP32 processor according to the provided documentation. Ensure that all the required connections and modules are correctly attached to the board. You can have your board easily made with the provided .7z file that you simply send to a PCB manufacturer. See Documentation for more.

- Software Setup: Install the necessary software and dependencies. This typically involves setting up the Visual Studio Code, Platformio IDE and the required libraries for programming the ESP32 board. Refer to the documentation for detailed instructions.

- Code Configuration: The code generally doesn't need modification to work. See Documentation on how the software is configured to see if you need to make any modifications.

- Upload the Code: Connect the custom board to your computer using a USB cable and upload the code to the ESP32 board using Platformio IDE the upload process for any errors and ensure successful code transfer.

- Testing and Integration: Once the code is uploaded, test the functionality of the board by interacting with the switches, devices, and modules connected to it. Verify that the digital inputs, outputs, MODBUS lines, and CAN line are working as intended.

- Customization and Expansion: Customize the code further to add additional features or integrate with other home automation systems. Expand the functionality of the project to suit your specific needs and preferences.

### Documentation
For detailed documentation, including hardware specifications, pin configurations, and usage guidelines, please refer to the Documentation folder in this repository.

### Contributing

Contributions to this home automation project are welcome! If you have ideas, suggestions, or improvements, please feel free to open an issue or submit a pull request. Let's collaborate and make home automation better together.

### License

This project is licensed under the GNU version 3.0 License. Feel free to use, modify, and distribute the code as per the terms of the license.
