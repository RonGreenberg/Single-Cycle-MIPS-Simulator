/*************************************************************************
*
* AUTHOR   : Ron Greenberg
* FILENAME : draw_syscalls.c
*
* Description:
* ------------
* The MIPS assembly doesn't provide support for VGA graphics, unlike x86 assembly for example, which allows switching to mode 13H for this purpose.
* In order to simulate this, as an "extension" to the MIPS architecture, we have created a separate Windows desktop application in C++ called BlankWindow.exe,
* using the Windows API. When running it, a blank window is shown. It listens for 7-byte UDP messages which follow a certain format (as shown in draw.c)
* composed of RGB and coordinate parameters, and upon receiving such a message, the application draws a rectangle on its screen according to the parameters.
* The files udp.h, udp.c provide an interface for sending messages to the server listening on the desktop app, while draw.h and draw.c provide useful
* functions for drawing a pixel, a rectangle or a whole bitmap represented using an array of bytes. These functions only require passing meaningful parameters,
* and they will take care of constructing the appropriate UDP message/s and sending them to the desktop app.
* In order to access this functionality from a MIPS assembly program running on our emulator, we defined 3 custom syscall codes: one for drawing a pixel, one for drawing
* a rectangle, and one for drawing a bitmap. Each of these syscalls receives its parameters via known registers (as specified in mipsdefs.h).
* Obviously, this will not work on MARS, but whenever our emulator identifies one of the custom syscall codes, the handle_draw_syscalls function, declared in this file,
* is invoked.
*
*************************************************************************/

#include "draw_syscalls.h"

void handle_draw_syscalls()
{
    // converting from unsigned long to unsigned char (no loss of data since all arguments are in range 0-255)
    unsigned char color = (unsigned char)registers[SYSCALL_DRAW_ARG1_REG];
    unsigned char x = (unsigned char)registers[SYSCALL_DRAW_ARG2_REG];
    unsigned char y = (unsigned char)registers[SYSCALL_DRAW_ARG3_REG];
    unsigned char width = (unsigned char)registers[SYSCALL_DRAW_ARG4_REG];
    unsigned char height = (unsigned char)registers[SYSCALL_DRAW_ARG5_REG];

    // first argument could also contain the bitmap array base address rather than color
    unsigned char *bitmap = (unsigned char *)data_mem;
    bitmap += (registers[SYSCALL_DRAW_ARG1_REG] % (DATA_MEM_SIZE * 4)); // advancing the pointer using the address specified in the first register

    // calling the appropriate DRAW function based on the syscall code
    switch (registers[SYSCALL_CODES_REG]) {
    case SYSCALL_CODE_DRAW_PIXEL:
        DRAW_pixel(color, x, y);
        break;
    case SYSCALL_CODE_DRAW_RECTANGLE:
        DRAW_rectangle(color, x, y, width, height);
        break;
    case SYSCALL_CODE_DRAW_BITMAP:
        DRAW_bitmap(bitmap, x, y, width, height);
        break;
    default:
        break;
    }
}
