# NiTTY - An Interpreter & REPL For STM32 Peripheral Control
This project was started as part of my dissertation during my undergraduate degree. It allows a user to interact with an STM32 Nucleo-F411RE Development Board. I developed a shell-style scripting language that allows the user to modify peripherals and GPIOs on the fly, without need for recompilation. The idea was simply a proof of concept, so the language lacks unimportant features such as variables, functions, loops, arrays, classes, etc. Don't expect it to do anything fancy. 

**This project will be open for pull requests at the end of my dissertation, at which point it will become FOSS. Forks are encouraged, as well as feedback!**

## Language Features
The scripting language lacks many features, but may come to include features considered "essential" to programming languages as I add to it. For now, the basic syntax can be seen below.

### Keywords
```
input <port/pin identifier> <pupd resistor config> -> Creates an GPI on the provided pin.
output <port/pin identifier> <pupd resistor config> -> Creates an GPO on the provided pin.
uart <RX port/pin identifier> <TX port/pin identifier> <baudrate> -> create a serial port on pins.
adc <port/pin identifier> -> create an ADC on the provided pin.

set <list of port/pin identifiers> -> Sets a list of GPIO pins.
reset <list of port/pin identifiers> -> Resets a list of GPIO pins.
toggle <list of port/pin identifiers> -> Toggles a list of GPIO pins.
read <list of port/pin identifiers> -> Reads a list of GPIO/ADC pins.

uart read -> read from the currently active UART port
uart write <string> -> write the string to the currently active UART port.
```

The line to be executed must always start with a keyword. As an example, say you want to set Port A pin 6 to be an input with a pulldown resistor:
```
input a06 pdown
```

Or that you want to set port B pin 11 to be an output with a pull up resistor:
```
output B11 pup
```

Say you want to set 2 pins to be outputs, and set them both high:
```
output a00 none
output a01 none
set a00 a01
```
To reset them again:
```
reset a00 a01
```
If you wanted to modify a pin function, you simply call the new function you want it to be, followed by its identifier and resistor config.

```
input a00 pdown
```
Pin A0 is now an input with a pulldown resistor.

## Building
Ensure you have the `arm toolchain` installed and is accessible from the shell PATH variable. This project makes use of `libopencm3`, which must be initialised and built before any modification can take place. Navigate to the `libopencm3` subdirectory, initialise the submodule and run `make`. After this is completed, both the bootloader and the firmware need to be built, which can be done via `make` in their respective directories. 

In order to flash the project to a development board, a program such as `st-utils` will be required. Settings for Visual Studio Code can be found in the `.vscode` directory.

This project was initially developed on GNU/Linux Debian 12 (bookworm) with kernel version 6.1.0. While it has not been tested on Windows/Mac, I assume it will work as long as you have Make, arm-gcc, and some way of flashing STM32 development boards. I will not be responding to any requests to get this working on other operating systems - this is an exercise for the reader!

## Using NiTTY
***Nitty is not finished***. It does not have loops, variables, conditionals, or any typically useful programming features (yet!). If you want to use it, I recommend using the Python wrapper provided in the `NiTTY-testing` repository (the file is called `nitty_wrapper.py`). This requires the NiTTY system to be connected by serial to a host computer running Python. When I get around to it, I'll make it into a proper Python package for ease of use. 