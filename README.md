# CH58x OTA Example
This is a PlatformIO CH582F OTA example project framework,  
which includes complete entry point jump, A/B partition, OTA Bootloader and other functional implementations.

# How to build

1. [Install PlatformIO](https://platformio.org)
2. Install **modified version** of ch32v platformio support *platform-ch58x-ota* via [PIO Core CLI](https://docs.platformio.org/en/latest/integration/ide/vscode.html#platformio-core-cli) --> `pio pkg install -g -p https://github.com/Angelic47/platform-ch58x-ota.git`
3. Run `pio run` to build the project.
4. Enjoy your hacking!

# License
This project is licensed under the Apache-2.0 license, as same as the original [CH58x BLE-USB-CDC-Example](https://github.com/Community-PIO-CH32V/platform-ch32v/tree/develop/examples/ble-usb-cdc-ch58x)
