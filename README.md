# arduino-ctcss-tone
Tone generator for Arduino
# How it works
Port D is set to all outputs, and all 8 bits can be set at once.
An array contains the values of a sine wave, scaled to [0, 255] . An interrupt service increments an index into the array, and outputs the value to port D.
Inside the loop() function, serial port is checked and if user enters a number, the counter inside the timer2 is changed to match the nearest possible frequency.
*Note* the counter is only 8 bit so only a limited range of frequencies is possible.
