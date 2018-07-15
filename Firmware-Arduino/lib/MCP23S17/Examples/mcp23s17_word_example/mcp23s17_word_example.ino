/*
  MCP23S17 Class Example
  Version 0.2
  Cort Buffington 2011
  
  This example will use the methods in "word mode", that is, writing/reading/setting
  16 bits at a time. Also note that on the inputs, we use the internal pull-up
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
  the methods being used in "bit mode", which replicates the utilitiy of arduino's
  built-in functions digitalWrite and digitalRead.
*/

#include <SPI.h>              // We use this library, so it must be called here.
#include <MCP23S17.h>         // Here is the new class to make using the MCP23S17 easy.

MCP inputchip(1, 10);             // Instantiate an object called "inputchip" on an MCP23S17 device at address 1
    							  // and slave-select on Arduino pin 10
MCP outputchip(2, 10);            // Instantiate an object called "outputchip" on an MCP23S17 device at address 2
  								  // and slave-select on Arduino pin 10

void setup() {
  inputchip.begin();
  outputchip.begin();
  inputchip.pinMode(0xFFFF);     // Use word-write mode to set all of the pins on inputchip to be inputs
  inputchip.pullupMode(0xFFFF);  // Use word-write mode to Turn on the internal pull-up resistors.
  inputchip.inputInvert(0xFFFF); // Use word-write mode to invert the inputs so that logic 0 is read as HIGH
  outputchip.pinMode(0x0000);    // Use word-write mode to Set all of the pins on outputchip to be outputs
}

void loop() {
  int value;                        // declare an integer to hold the value temporarily.
  value = inputchip.digitalRead();  // read the input chip in word-mode, storing the result in "value"
  outputchip.digitalWrite(value);   // write the output chip in word-mode, using our variable "value" as the argument
  // outputchip.digitalWrite(inputchip.digitalRead()); // this one line replaces the three above, and is more efficient
}
