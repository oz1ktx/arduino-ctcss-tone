# arduino-ctcss-tone
Tone generator for Arduino
# How it works
Port D is set to all outputs, and all 8 bits can be set at once.
An array contains the values of a sine wave, scaled to [0, 255] . An interrupt service increments an index into the array, and outputs the value to port D.
Inside the loop() function, serial port is checked and if user enters a number, the counter inside the timer1 is changed to match the nearest possible frequency.
# Persistent data
Every time a valid frequency is entered, it is saved to EEPROM so the tone generator will start on the same frequency next time.
