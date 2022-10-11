/*************************************************************************
*
* AUTHOR   : Ron Greenberg
* FILENAME : main.c
*
* Description:
* ------------
* Main file to test the emulator.
*
*************************************************************************/

#include "mips.h"
#include "draw.h"

int main(void) {
    int finished = 0;
    MIPS_info_t mips_info;

    MIPS_init("fibonacci_data.hex", "fibonacci_prog.hex");
    MIPS_get_info(&mips_info);

    DRAW_init(); // initializing DRAW module (which initializes the UDP module)

    // overwriting program memory to manually set a program
#if 0
    *(mips_info.prog_size) = 18;
    unsigned long *prog = mips_info.prog_mem_base;
    memset(prog, 0, PROG_MEM_SIZE * sizeof(unsigned long));
    prog[0]  = 0x20030007; // addi   r3, r0, 7
    prog[1]  = 0x00602024; // and    r4, r3, r0
    prog[2]  = 0x00032880; // sll    r5, r3, 2
    prog[3]  = 0x20060004; // addi   r6, r0, 4
    prog[4]  = 0x200c001C; // addi   r12,r0, 28
    prog[5]  = 0x200a0001; // addi   r10,r0, 1
    prog[6]  = 0x20090000; // addi   r9, r0, 0
    prog[7]  = 0x8d210008; // lw     r1, 8($r9)
    prog[8]  = 0x8d22004C; // lw     r2, 76($r9)     # should contain the value of "size": 19 = 0x13
    prog[9]  = 0x0022582a; // slt    r11,r1, r2      # r11 should become 1
    prog[10] = 0x24017fff; // addiu  r1, r0, 0x7fff  # using maximum 16-bit signed number
    prog[11] = 0x70411002; // mul    r2, r2, r1      # r2 should become 0x97fed
    prog[12] = 0x00220019; // multu  r1, r2          # hi=0x4, lo=0xbfed0013
    prog[13] = 0xad210008; // sw     r1, 8($r9)
    prog[14] = 0xad220004; // sw     r2, 4($r9)
    prog[15] = 0x7435d082; // fake instruction (prints Unsupported instruction)
    prog[16] = 0x2402000a; // li     $v0, 10         # system call for exit
    prog[17] = 0x0000000c; // syscall
    // any additional instruction will not be executed since we exit
#endif

    while (!finished) {
        //printf("Instruction #%ld\n", (*(mips_info.pc) >> 2) % PROG_MEM_SIZE);
        finished = MIPS_step();
        //printf("%08x\n", mips_info.prog_mem_base[*(mips_info.pc) >> 2]); // printing current instruction (in hex with fixed length of 8)
        /*for (int i = 0; i < 32; i++) {
            printf("Register: %d. Value: %lx\n\r", i, mips_info.reg_mem_base[i]);
        }
        printf("hi=%lx, lo=%lx\n\n", *(mips_info.hi), *(mips_info.lo));*/
    }

    DRAW_terminate();

#if 0
    char *byte_ptr = (char *)mips_info.data_mem_base;
    printf("%x\n", byte_ptr[0xdc]);

    long *word_ptr = mips_info.data_mem_base;
    printf("%x\n", word_ptr[0xdc >> 2]);

    short *hw_ptr = (short *)mips_info.data_mem_base;
    printf("%x", hw_ptr[0xdc >> 1]);
#endif

#if 0
    unsigned long value = 0x0000ffffL;
    char *byte_ptr = (char *)&value;
    printf("%x", (unsigned char)*byte_ptr); // without casting to unsigned char, sign extension would be performed (because there is type promotion to int) and ffffffff would be printed
#endif

#if 0
    unsigned long hi, lo;
    long signed_src1 = -5, signed_src2 = -2;
    lo = signed_src1 / signed_src2;
    hi = signed_src1 % signed_src2;

    printf("%d %d %d %d\n\r", signed_src1, signed_src2, lo, hi);
    exit(0);
#endif

#if 0
    unsigned long result = 0xFFFFFF50;
    long signed_src1, signed_src2 = 0xFFFFFF50;
    result = signed_src2 >> 4;

    printf("%08x\n\r", result);
    exit(0);
#endif

#if 0
    unsigned long hi, lo;
    long signed_src1, signed_src2;
    long long mult_result = 0xf000000000000005LL;
    lo = (unsigned long)mult_result; // 0xf0000000,00000005
    hi = (unsigned long)(mult_result >> 32);

    printf("%08x %08x\n\r", hi, lo);
    exit(0);
#endif

#if 0
    short imm = 0x7fff;
    long signed_src2 = imm;
    printf("%lx\n", signed_src2);
    printf("%lx\n", signed_src2 << 16);
    long signed_src1 = 0xffff0000;
    printf("%lx\n", signed_src1 * signed_src2);
#endif

#if 0
    signed int x1 = 5;
    printf("%d\n", (x1 >> 1) == 2);
    signed int x2 = -5;
    printf("%d\n", (x2 >> 1) == -3);
    unsigned int x3 = (unsigned int)-5;
    printf("%d\n", (x3 >> 1) == 0x7FFFFFFD);
#endif

    return 0;
}
