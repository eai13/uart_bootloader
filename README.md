# UART Bootloader

`./Core` - Source code for STM32G071RB bootloader

`./Serial_firm` - Source code for QT application flasher

### Bootloader description

|Boot Command|What?|
|:----------:|:---:|
|PING        |Verifies device connection, gets device ID|
|ERASE       |Erases the application pages of memory|
|JUMP        |Jumps to the main application|
|READ        |Does nothing yet|
|WRITE       |Sequentially writes the firmware to the controller|

### Protocol

Protocol consists of a header and sometimes payload (only needed in write command)

Header structure

|Size|Field|What?|
|:--:|:---:|:---:|
|4 bytes|device_src|Sender Device ID|
|4 bytes|device_dest|Destination Device ID|
|1 byte|cmd|Boot command|
|4 bytes|payload_size|Amount of data upcoming next|

During booting the device returns `BOOT_OK` in all cases of successful operations.
In cases of errors the device returns `BOOT_ERROR`.

### Examples for each command type

**PING example**

*REQUEST*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"HOST"|"DEVC"|PING|0|

*RESPONSE*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"DEVC"|"HOST"|BOOT_OK|0|

**ERASE example**

**PING example**

*REQUEST*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"HOST"|"DEVC"|ERASE|0|

*RESPONSE*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"DEVC"|"HOST"|BOOT_OK|0|

**JUMP example**

**PING example**

*REQUEST*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"HOST"|"DEVC"|JUMP|0|

*RESPONSE*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"DEVC"|"HOST"|BOOT_OK|0|

**WRITE example**

**PING example**

*REQUEST*

Write commands must be sent sequentially, limited at 256 byte chunks each payload. In case of error, or sending some other command,
the address register falls back to the beginning of the application FLASH. 

|device_src|device_dest|cmd|payload_size|data|
|:-:|:-:|:-:|:-:|:-:|
|"HOST"|"DEVC"|WRITE|256|Chunk of Data (256 bytes)|

*RESPONSE*

|device_src|device_dest|cmd|payload_size|
|:-:|:-:|:-:|:-:|
|"DEVC"|"HOST"|BOOT_OK|0|

After writing a chunk of firmware to the FLASH the device responds with `BOOT_OK`. This means, it is ready for a new chunk to receive.
