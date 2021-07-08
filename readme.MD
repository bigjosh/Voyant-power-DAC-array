# Voyant Power DAC Array

Provides an array of digitally controllable power supplies that can each provide up to 10V at 1A with 12 bits of resolution.

## Theory of operation

We use a stack of Visgence Power DAC shields and an Arduino Uno as our base. 

Stock these shields are bi-polar and can run -15V to +15. They also only have 4 chip select lines.  

We modify the shields to make them single ended and able to go up to 30V and also add more chip select lines so we can have more connected to a single Arduino at the same time.

Then we load some software on the Arduino that accepts simple command over the serial port to set the output voltages of the shields.

## DAC shield modifications

The stock schematic is [here](dac_shield.pdf).

We adapt it to look like this...

![](dac_shield_schematic_modified.png)

Basically we are...

1. Changing the DAC to use the Arduino 5V as VREF. This will let it go full scale and give us maximum resolution. 
2. Moving the output of the DAC to feed directly into the positive 2nd stage of the output amp.
3. Changing that 2nd stage output amp into a variable gain amplifier with 2x gain in the center of the adjustment POT. 

## Shield rework steps

1. Remove U3 (741) completely from the socket. We will not use this chip
2. Add a jumper between U1 pin 1 and pin 6 on the bottom side of the PCB
3. Cut the trace between U2 pin 1 and pin 6 on the bottom side of the PCB  
3. Add a jumper between U2 pin 7 and pin 6 on the bottom side of the PCB
4. Short out R5 and R6 with jumpers or 0 ohm resistors

   
    