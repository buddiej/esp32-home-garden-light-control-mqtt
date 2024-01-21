# esp32-home-garden-light-control-mqtt
with home garden light it is possible to control 18 LED's independently. It is possible to set brightness and status. Result of control get gaet back. Control is done via JSON and MQTT.

It uses wire. with higer buffer size. See https://github.com/Justin-Pl/PCA9685_LED_DRIVER how to configure.

#I2C Buffer problem
The Wire.h library uses a 32 byte buffer for some boards like the Arduino Nano. This library send data packages larger than 32 bytes to the PCA9685.

Close the Arduino IDE if open.

Find the file Wire.h:

Windows (IDE 2): C:\Users{username}\AppData\Local\Arduino15\packages\arduino\hardware\avr{version}\libraries\Wire\src\Wire.h
Windows (IDE 1.x): C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\Wire\src\Wire.h
macOS: ~/Library/Arduino15/hardware/arduino/avr/libraries/Wire/src/Wire.h
Linux: ~/sketchbook/hardware/arduino/avr/libraries/Wire/src/Wire.h
Open the file Wire.h with a text editor.

Locate the line #define BUFFER_LENGTH 32 and change the number 32 to the desired value(128).

Save the changes made and close the text editor.

Restart the IDE.

