/*************************************************************************
*
* AUTHOR   : Ron Greenberg
* FILENAME : mips.h
*
* Description:
* ------------
* This file defines some constants, a structure used to access info through main, and declarations of functions that need to be accessible from main.
*
*************************************************************************/

#ifndef __MIPS_H
#define __MIPS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h> // for sleep function
#include "mipsdefs.h"

#define DATA_MEM_SIZE 1024
#define PROG_MEM_SIZE 1024

// externing registers and data memory for use in draw_syscalls.c
extern unsigned long registers[NUM_REG];
extern unsigned long data_mem[DATA_MEM_SIZE];

// structure used for debugging
typedef struct {
    unsigned long *prog_mem_base;
    unsigned long *data_mem_base;
    unsigned long *reg_mem_base;
    unsigned long *pc;
    unsigned long *alu_res;
    unsigned long *hi, *lo;
    unsigned int  *prog_size;
} MIPS_info_t;

/* This function receives two filenames representing:
   1. A text file containing the entire .data segment as 32-bit hex values separated across lines. The non-zero values appearing there will be the ones
      declared and initialized under the .data directive/s in the .asm file.
   2. A text file containing the assembled program as 32-bit hex values separated across lines, representing the instructions written under the .text directive/s
      in the .asm file.
   You can generate both of these files using MARS, but before doing so go to Settings->Memory Configuration... and make sure that the selected configuration is
   "Compact, Data at Address 0". That's because we store the memory data as an array read from the dumped data file, and use the offset addresses read from
   instructions in the program file (calculated by the assembler that generated the file) as the indexes to access this array.
   For example, in the Default memory configuration, the .data base address is 0x10010000, so if the la (load address) instruction needs to load an address found
   76 bytes after the beginning of the data segment, then the loaded address will be 0x1001004C. We cannot later use such an address to load the data stored there,
   because it would go out of range, as array indexes obviously start at 0. So we just need to make sure that the data segment starts at address 0 as well.
*/
void MIPS_init(const char *data_filename, const char *program_filename);

// This function emulates the entire processor operation for a single instruction. It returns 1 if the program is finished (determined solely by reaching an exit syscall), and 0 otherwise.
int MIPS_step(void);

// this function receives the address of an info object and updates its contents
void MIPS_get_info(MIPS_info_t *info);

#endif /* __MIPS_H */
