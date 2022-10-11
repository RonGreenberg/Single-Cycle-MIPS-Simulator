/*************************************************************************
*
* AUTHOR   : Ron Greenberg
* FILENAME : mipsdefs.h
*
* Description:
* ------------
* This file contains opcodes and funct values for the MIPS instruction set, syscall codes, instruction structs and constants.
* Pseudo-instructions are not included - instructions that do not have a direct hardware implementation and are provided as a convenience for the programmer.
* These include: move, li, la, blt, ble, bgt, bge.
* They are translated by the assembler into equivalent real MIPS instructions (might be composed of multiple, for specific pseudo-instructions), included in this file.
* For example, MARS translates the instruction: li $rs, imm to: addiu $rs, $zero, imm.
*
*************************************************************************/

#ifndef __MIPSDEFS_H
#define __MIPSDEFS_H

#define NUM_REG 32 // number of registers in the register file
#define RESET_ADDR 0x3000 // program counter reset address (same as .text base address in MARS's "Compact, Data at Address 0" memory configuration)

// Opcodes
#define OPCODE_RTYPE    0x00 // all R-type instructions have an opcode of 0, and they are differentiated by their funct values (see below)
#define OPCODE_J        0x02 // j lbl           :  pc += i << 2
#define OPCODE_JAL      0x03 // jal lbl         :  $31 = pc; pc += i << 2
#define OPCODE_BEQ      0x04 // beq $s, $t, lbl :  if ($s == $t) pc += i << 2
#define OPCODE_BNE      0x05 // bne $s, $t, lbl :  if ($s != $t) pc += i << 2
#define OPCODE_BLEZ     0x06 // blez $s, lbl    :  if ($s <= 0) pc += i << 2
#define OPCODE_BGTZ     0x07 // bgtz $s, lbl    :  if ($s > 0) pc += i << 2
#define OPCODE_ADDI     0x08 // addi $t, $s, i  :  $t = $s + SignExt(i)
#define OPCODE_ADDIU    0x09 // addiu $t, $s, i :  $t = $s + SignExt(i)
#define OPCODE_SLTI     0x0A // slti $t, $s, i  :  $t = ($s < SignExt(i))
#define OPCODE_SLTIU    0x0B // sltiu $t, $s, i :  $t = ($s < SignExt(i))
#define OPCODE_ANDI     0x0C // andi $t, $s, i  :  $t = $s & ZeroExt(i)
#define OPCODE_ORI      0x0D // ori $t, $s, i   :  $t = $s | ZeroExt(i)
#define OPCODE_XORI     0x0E // xori $t, $s, i  :  $t = $s ^ ZeroExt(i)
#define OPCODE_LUI      0x0F // lui $t, i       :  $t = (i) << 16 || 0x0000
#define OPCODE_SPECIAL2 0x1C // SPECIAL2 mode (to support the mul instruction, which is NOT a pseudo-instruction)
#define OPCODE_LB       0x20 // lb $t, i($s)    :  $t = SignExt(MEM[$s + i]:1)
#define OPCODE_LH       0x21 // lh $t, i($s)    :  $t = SignExt(MEM[$s + i]:2)
#define OPCODE_LW       0x23 // lw $t, i($s)    :  $t = MEM[$s + i]:4
#define OPCODE_LBU      0x24 // lbu $t, i($s)   :  $t = ZeroExt(MEM[$s + i]:1)
#define OPCODE_LHU      0x25 // lhu $t, i($s)   :  $t = ZeroExt(MEM[$s + i]:2)
#define OPCODE_SB       0x28 // sb $t, i($s)    :  MEM[$s + i]:1 = LowerByte($t)
#define OPCODE_SH       0x29 // sh $t, i($s)    :  MEM[$s + i]:2 = LowerHalfword($t)
#define OPCODE_SW       0x2B // sw $t, i($s)    :  MEM[$s + i]:4 = $t

// R-type funct values
#define FUNCT_SLL     0x00 // sll $d, $t, a   :  $d = $t << a
#define FUNCT_SRL     0x02 // srl $d, $t, a   :  $d = $t >>> a
#define FUNCT_SRA     0x03 // sra $d, $t, a   :  $d = $t >> a
#define FUNCT_SLLV    0x04 // sllv $d, $t, $s :  $d = $t << $s
#define FUNCT_SRLV    0x06 // srlv $d, $t, $s :  $d = $t >>> $s
#define FUNCT_SRAV    0x07 // srav $d, $t, $s :  $d = $t >> $s
#define FUNCT_JR      0x08 // jr $s           :  pc = $s
#define FUNCT_JALR    0x09 // jalr $s         :  $31 = pc; pc = $s
#define FUNCT_SYSCALL 0x0C // syscall
#define FUNCT_BREAK   0x0D // break
#define FUNCT_MFHI    0x10 // mfhi $d         :  $d = hi
#define FUNCT_MTHI    0x11 // mthi $s         :  hi = $s
#define FUNCT_MFLO    0x12 // mflo $d         :  $d = lo
#define FUNCT_MTLO    0x13 // mtlo $s         :  lo = $s
#define FUNCT_MULT    0x18 // mult $s, $t     :  hi:lo = $s * $t
#define FUNCT_MULTU   0x19 // multu $s, $t    :  hi:lo = $s * $t
#define FUNCT_DIV     0x1A // div $s, $t      :  lo = $s / $t; hi = $s % $t
#define FUNCT_DIVU    0x1B // divu $s, $t     :  lo = $s / $t; hi = $s % $t
#define FUNCT_ADD     0x20 // add $d, $s, $t  :  $d = $s + $t
#define FUNCT_ADDU    0x21 // addu $d, $s, $t :  $d = $s + $t
#define FUNCT_SUB     0x22 // sub $d, $s, $t  :  $d = $s - $t
#define FUNCT_SUBU    0x23 // subu $d, $s, $t :  $d = $s - $t
#define FUNCT_AND     0x24 // and $d, $s, $t  :  $d = $s & $t
#define FUNCT_OR      0x25 // or $d, $s, $t   :  $d = $s | $t
#define FUNCT_XOR     0x26 // xor $d, $s, $t  :  $d = $s ^ $t
#define FUNCT_NOR     0x27 // nor $d, $s, $t  :  $d = ~($s | $t)
#define FUNCT_SLT     0x2A // slt $d, $s, $t  :  $d = ($s < $t)
#define FUNCT_SLTU    0x2B // sltu $d, $s, $t :  $d = ($s < $t)

// SPECIAL2 Opcode functs
#define FUNCT_MUL     0x02 // mul $d, $s, $t  :  $d = $s * $t (least 32 bit)

#define SYSCALL_CODES_REG 2 // the syscall code must be stored in register 2 ($v0) before executing syscall
#define SYSCALL_ARG1_REG  4 // the argument to print_int, print_string, print_char, sleep, print_int_hex, print_int_bin, print_uint is stored in $a0 (register 4)

// SYSCALL Codes
#define SYSCALL_CODE_PRINT_INT     1   // $a0 (reg 4) = integer to print
#define SYSCALL_CODE_PRINT_STRING  4   // $a0 (reg 4) = address of null terminated string
#define SYSCALL_CODE_READ_INT      5   // $v0 (reg 2) will contain result
#define SYSCALL_CODE_EXIT          10  // end program
#define SYSCALL_CODE_PRINT_CHAR    11  // $a0 (reg 4) contains the char
#define SYSCALL_CODE_SLEEP         32  // $a0 (reg 4) = the length of time to sleep in milliseconds
#define SYSCALL_CODE_PRINT_INT_HEX 34  // Prints int in hex format. $a0 (reg 4) = integer to print
#define SYSCALL_CODE_PRINT_INT_BIN 35  // Prints int in binary format. $a0 (reg 4) = integer to print
#define SYSCALL_CODE_PRINT_UINT    36  // Prints unsigned integer. $a0 (reg 4) = integer to print

// custom SYSCALL codes for drawing on virtual screen
#define SYSCALL_CODE_DRAW_PIXEL     18 // $t0 (reg 8) = color, $t1 = x, $t2 = y
#define SYSCALL_CODE_DRAW_RECTANGLE 19 // $t0 (reg 8) = color, $t1 = x, $t2 = y, $t3 = width, $t4 = height
#define SYSCALL_CODE_DRAW_BITMAP    20 // $t0 (reg 8) = bitmap array base address, $t1 = x, $t2 = y, $t3 = width, $t4 = height
// registers for arguments to the custom syscalls
#define SYSCALL_DRAW_ARG1_REG       8  // $t0
#define SYSCALL_DRAW_ARG2_REG       9  // $t1
#define SYSCALL_DRAW_ARG3_REG       10 // $t2
#define SYSCALL_DRAW_ARG4_REG       11 // $t3
#define SYSCALL_DRAW_ARG5_REG       12 // $t4


// defining structures using bitfields to access the instructions parts more easily

// R-type instructions
struct r_type {
    unsigned funct : 6; // funct - 6 bits
    unsigned shamt : 5; // shamt (shift amount) - 5 bits
    unsigned rd : 5; // rd - 5 bits
    unsigned rt : 5; // rt - 5 bits
    unsigned rs : 5; // rs - 5 bits
    unsigned opcode : 6; // op-code - 6 bits
};

// I-type (immediate) instructions
struct i_type {
    unsigned addr_im : 16; // address/immediate
    unsigned rt : 5; // rt
    unsigned rs : 5; // rs
    unsigned opcode : 6; // op-code
};

// J-type (jump) instructions
struct j_type {
    unsigned addr : 26; // target address
    unsigned opcode : 6; // op-code
};

// common type used for semantics (so that we don't "assume" the instruction type when accessing the opcode)
struct common_type {
    unsigned dummy : 26;
    unsigned opcode : 6; // all types of instructions start with an opcode of 6 bits
};

/* In a union, all members share the same memory location. We can notice that all members in this union have a size of exactly 32 bits. It means that the whole union
   size will be 32 bits, shared by all members, so that whenever we set the value of one of them, all others will change as well.
   Using this, after we determine the instruction type, we can easily choose it among the members and rely on its partition of bits to access the relevant parts.
*/
typedef union {
    struct r_type rtype;
    struct i_type itype;
    struct j_type jtype;
    struct common_type commontype;
    unsigned long inst;
} instruction_t;

#endif /* __MIPSDEFS_H */
