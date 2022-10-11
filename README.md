# Single-Cycle-MIPS-Simulator
A simulator for running programs written in Assembly language of the Single-Cycle [MIPS architecture](https://en.wikipedia.org/wiki/MIPS_architecture), as described in Computer Organization and Design 5th Edition, pages 251-271.  
Supports the basic [instruction set](https://inst.eecs.berkeley.edu/~cs61c/resources/MIPS_help.html), can run essentially any program that the [MARS simulator](http://courses.missouristate.edu/kenvollmar/mars/) is capable of running.  
It also provides an "extension" for graphics, similar to mode 13H of x86 Assembly, which allows the user to draw pixels, rectangles or whole bitmaps by simply using custom syscall codes as part of the assembly program.  
This is achieved by communicating with an external program I've written for this purpose, BlankWindow, over UDP. It displays a window which serves as a canvas for drawing.
## Folder structure
- `BlankWindow`: contains the source code and executable program of BlankWindow.
- `resources`: contains example assembly programs tested on the simulator, including one that demonstrates the use of graphics.  
It also contains two additional text files for each program, which are the ones actually fed to the simulator - one containing the entire .data segment of the program, and the other containing the assembled program instructions (.text segment). Both files contain 32-bit hex values separated across lines. They can be generated using [MARS](http://courses.missouristate.edu/kenvollmar/mars/) upon finishing writing a program.
- The main folder contains the simulator source code:
    - `mipsdefs.h`: contains opcodes and funct values for the MIPS instruction set, syscall codes, instruction structs and constants.
    - `mips.h` and `mips.c` contain the implementation of the MIPS single-cycle datapath, including the Control unit, ALU, Register file, Instruction memory and Data memory.
    - `udp.h` and `udp.c` provide an interface for sending UDP messages to the server listening on the BlankWindow desktop app, using Winsock.
    - `draw.h` and `draw.c` provide functions for drawing a pixel, a rectangle or a whole bitmap represented using an array of bytes. These functions take care of constructing the appropriate UDP message/s and sending them to the BlankWindow desktop app.
    - `draw_syscalls.h` and `draw_syscalls.c` map the special graphics syscalls to the draw functions they are meant to invoke.
    - `main.c` contains the main program to test the simulator.
