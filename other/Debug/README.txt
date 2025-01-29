To use esp-prog you must first replace the driver via Zadig

1) Run Zadig
2) Options->List all devices
3) Select Dual RS232-HS (Interface 0)
4) Replace Driver with WinUSB

Once done you can start using the debugger via PlatformIO's "Run and Debug"


To JTAG debug ESP32-S3 it is generally easiest to simply use the builtin debugger. If that is not possible an external debugger can be used
To use an external debugger an efuse must be burned. This is done by
- installing ESP-IDF
- running the CMD environment (shows up as ESP-IDF(CMD) in Windows search)
- reading and/or checking connection by running "espefuse.py --port COM16 summary" (change COM)
- burning the fuse with "espefuse.py -p COM16 burn_efuse DIS_USB_JTAG"