/*
  MCP23S17 Class Example
  Version 0.2
  Cort Buffington 2013
  
  This example will use the methods in "bit mode", that is, writing/reading/setting
  bit at a time. Also note that on the inputs, we use the internal pull-up
  resistors and an SPST button to ground when the button is pushed. While this uses
  simpler (less expensive) pushbuttons, it does invert the logic. This is a perfect
  example of one of the more advanced features of the MCP23S17 where we can invert
  the logic state of the input pins. So, when a button is closed, we read a logic
  low state, but the chip will report it as a logic high.
  
  In the example, we'll set up two chips, one as an input and one as an output. We
  will read the input chip, then write the input pin states to the output chip. This
  is a completely useless program in the real world, but it demonstrates the
  abilities of this class well, and makes a simple example.
  
  A few things are done less efficiently than possible to make the sketch easier to
  follow. In the hardware setup I used to write this example, I set up 2 x 8 position
  DIP switches on the input chip, and 16 LEDs on the output chip.
  
  Be sure to see the companion example that demonstrates the same funcionality with
  the methods being used in "word mode", which is significantly more efficient if you
  need to read or write an entire word at one time.
*/

#include <SPI.h>              // We use this library, so it must be called here.
#include <MCP23S17.h>         // Here is the new class to make using the MCP23S17 easy.

MCP inputchip(1, 10);             // Instantiate an object called "inputchip" on an MCP23S17 device at address 1
  								  // and slave-select on Arduino pin 10
MCP outputchip(2, 10);            // Instantiate an object called "outputchip" on an MCP23S17 device at address 
  								  // and slave-select on Arduino pin 10

void setup() {
  inputchip.begin();
  outputchip.begin();
  for (int i = 1; i <= 16; i++) {    // Since we are only workign with one bit at a time, use a loop to take action each pin (0-15)
    inputchip.pinMode(i, HIGH);      // Use bit-write mode to set all of the current pin on inputchip to be inputs
    inputchip.pullupMode(i, HIGH);   // Use bit-write mode to Turn on the internal pull-up resistor for the current pin
    inputchip.inputInvert(i, HIGH);  // Use bit-write mode to invert the input so that logic 0 is read as HIGH
    outputchip.pinMode(i, LOW);      // Use bit-write mode to Set all of the current pin on outputchip to be an output
  }
}

void loop() {
  for (int i = 1; i <=16; i++) {       // Step through each of the 16 pins on the I/O chips
    int value;                         // declare an integer to hold the value temporarily.
    value = inputchip.digitalRead(i);  // read the input chip in bit-mode, storing the result for bit "i" in "value"
    outputchip.digitalWrite(i, value); // write the output chip in bit-mode, using our variable "value" as the argument for bit "i"
    // outputchip.digitalWrite(i, inputchip.digitalRead(i)) // this one line replaces everything above except the for loop, and is more efficient
  }
}
