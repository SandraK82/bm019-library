# bm019-library

Library project to interface a bm019 module ( http://www.solutions-cubed.com/bm019/ ) (based on ST Micro CR95HF NFC interface chip ( http://www2.st.com/content/st_com/en/products/memories/nfc-rfid-memories-and-transceivers/nfc-rfid-transceivers/cr95hf.html ) ).

This is a work in progress project where i will add functions and support if needed.

The primary function is to drive an arduino, esp8266, or nRF51 based nfc to serial, wifi, or ble systems in conjuction with an open reader library for the Abbott Freestyle Libre NFC based flash clucose measurement system. (more projects will appear soon)

This Library can be included via mbed, choose between a spi or uart version: https://developer.mbed.org/users/SandraK/code/BLE-bm019-spi/
https://developer.mbed.org/users/SandraK/code/BLE-bm019-uart/

The spi version is preconfigured for the nrf51-dk evaluation board.
The Wiring is:
- BM019 nrf51-DK
- VIN   5V
- GND   Ground
- VDD-SS0 (bridge VDD (3V) to SS0 to indicate SPI MOde to BM019)
- DIN   P0.01
- SS    P0.02
- MISO  P0.03
- MOSI  P0.04
- CLK   P0.05

The uart version is preconfigured for the Bluefruit UART friend:
- Power     -> BM019.VIN
- Ground    -> BM019.SS0, BM019.GND, BLE.GND
- BM019.VDD -> BLE.VIN
- BM019.DIN -> BLE.TX
- BM019.DOUT -> BLE.RX

We have completly erased the bluefruit and overwritten with std bootloader, softdevice and our firmware via nrf51-dk onboard programmer.
