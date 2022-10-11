/*************************************************************************
*
* AUTHOR   : Ron Greenberg
* FILENAME : mips.c
*
* Description:
* ------------
* This file contains the implementation of the MIPS single-cycle datapath, including the Control unit, ALU, Register file, Instruction memory and Data memory.
*
*************************************************************************/

#include "mips.h"
#include "draw_syscalls.h"

unsigned long data_mem[DATA_MEM_SIZE];
unsigned long prog_mem[PROG_MEM_SIZE];
unsigned long registers[NUM_REG]; // register file
unsigned long hi, lo; // hi, lo registers for multiplication/division results (since they're not among the first 32 registers)
unsigned long pc; // program counter (counts bytes, not words)
instruction_t current_instruction;
unsigned int prog_size; // contains the actual number of instructions in the program
unsigned long alu_result;

struct control_t {
    // 1-bit control signals + alu_op
    unsigned branch : 1; // determines whether to possibly branch to some target (1) or continue to the next instruction (pc + 4) as usual (0)
    unsigned jump : 1; // this signal is turned on, only when the instruction is j/jal
    unsigned jump_register : 1; // this signal is turned on, only when the instruction is jr/jalr
    unsigned jump_and_link : 1; // this signal is turned on, only when the instruction is jal/jalr
    unsigned reg_dest : 1; // determines whether to take the register destination number for the Write register from rt (0), or from rd (1)
    unsigned reg_write : 1; // if it's on, the register on the Write register input is written with the value on the Write data input
    unsigned alu_src : 1; // determines whether to take the second ALU operand from the second register file output (0), or from the sign-extended, lower 16 bits of the instruction (1)
    unsigned alu_op : 6; // 6-bit field containing the operation that the ALU needs to perform
    unsigned mem_write : 1; // if it's on, the data memory contents designated by the address input are replaced by the value on the Write data input
    unsigned mem_read : 1; // if it's on, the data memory contents designated by the address input are put on the Read data output
    unsigned mem_to_reg : 1; // determines whether to take the value for the register Write data input from the ALU (0), or from the data memory (1)
} control; // also defines a control variable

// function used for debugging via main. when someone asks for the information, we update the members using the global variables in this file
void MIPS_get_info(MIPS_info_t *info)
{
    info->prog_mem_base = prog_mem;
    info->data_mem_base = data_mem;
    info->reg_mem_base = registers;
    info->pc = &pc;
    info->alu_res = &alu_result;
    info->hi = &hi;
    info->lo = &lo;
    info->prog_size = &prog_size;
}

// this function fills the passed array with the contents of the file with the specified filename
int read_file_to_memory(const char *filename, unsigned long *mem)
{
    FILE *fptr;
    char buffer[10]; // 8 hex digits + newline char + null terminator
    int i = 0;

    fptr = fopen(filename, "r"); // opening as a text file
    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0'; // removing the trailing newline character added to the buffer by fgets (strcspn finds the index of the first \n)
        mem[i] = strtoul(buffer, NULL, 16); // converting hex string to long using strtoul (string to UNSIGNED long) function from stdlib
        i++;
    }
    fclose(fptr);

    return i; // returning the number of lines read
}

void MIPS_init(const char *data_filename, const char *program_filename)
{
    int i;

    read_file_to_memory(data_filename, data_mem);
    prog_size = read_file_to_memory(program_filename, prog_mem);

    pc = RESET_ADDR;

    // clearing registers
    for (i = 0; i < NUM_REG; i++) {
        registers[i] = 0;
    }
    hi = 0;
    lo = 0;
}

void generate_control(void)
{
    // initial control signals
    control.branch = 0;
    control.jump = 0;
    control.jump_register = 0;
    control.jump_and_link = 0;
    control.reg_dest = 1;
    control.reg_write = 1;
    control.alu_src = 0;
    control.alu_op = 0;
    control.mem_write = 0;
    control.mem_read = 0;
    control.mem_to_reg = 0;

    if (current_instruction.commontype.opcode == OPCODE_RTYPE) {
        control.alu_op = current_instruction.rtype.funct; // for all R-type instructions, alu_op is the funct value

        // handling special cases
        switch (current_instruction.rtype.funct) {
        case FUNCT_JR:
            control.jump_register = 1;
            control.reg_write = 0; // this instruction should not write to the register given to it
            break;
        case FUNCT_JALR: // note that reg_write is not disabled, because this instruction writes the incremented PC to $ra (31)
            control.jump_register = 1;
            control.jump_and_link = 1;
            break;
        case FUNCT_SYSCALL:
        case FUNCT_BREAK:
            control.reg_write = 0; // syscall/break do not write to a register
            break;
        case FUNCT_MTHI:
        case FUNCT_MTLO:
        case FUNCT_MULT:
        case FUNCT_MULTU:
        case FUNCT_DIV:
        case FUNCT_DIVU:
            control.reg_write = 0; // the instructions in this group write to the hi and lo registers, but not to the registers given to them
            break;
        default:
            break;
        }
    } else {
        // common signal values applying for MOST supported non R-type instructions
        control.reg_dest = 0; // $rt should be written to the regfile, not $rd
        control.alu_src = 1; // the second ALU operand should come from the immediate value of the instruction

        switch (current_instruction.commontype.opcode) { // J-type or I-type
        case OPCODE_J:
            control.reg_write = 0; // this instruction should not write to the register given to it
            control.jump = 1;
            break;
        case OPCODE_JAL: // note that reg_write is not disabled, because this instruction writes the incremented PC to $ra (31)
            control.jump = 1;
            control.jump_and_link = 1;
            break;
        case OPCODE_BEQ:
        case OPCODE_BNE:
            // note that we do not set the branch signal here because it depends on the ALU result
            control.reg_write = 0;
            control.alu_src = 0; // in these instructions, the second ALU operand must come from the second register file output
            control.alu_op = FUNCT_SUB; // we need to subtract the register values and check whether or not the ALU result is 0
            break;
        case OPCODE_BLEZ:
        case OPCODE_BGTZ:
            // again - not setting the branch signal here, but unlike beq and bne, here we don't even care about the ALU result. The logic was moved to branch_control
            control.reg_write = 0;
            break;
        case OPCODE_ADDI:
            control.alu_op = FUNCT_ADD;
            break;

        /* The term "unsigned" in the addiu instruction name is a misnomer, because the immediate is still sign extended instead of zero extended. It means that the
           immediate argument must be a 16-bit signed number, so the maximum is 32767 (0x7fff). MARS's assembler also supports passing a 16-bit unsigned immediate,
           but in that case it treats addiu as a pseudo-instruction and translates addiu $t, $s, unsigned_imm to the following instructions:
           lui $at, 0
           ori $at, $at, ZeroExt(unsigned_imm)
           addu $t, $s, $at
           MARS also does this for addi.
           In fact the only difference between addi/add and addiu/addu is that addi/add generates a trap on overflow (also applies for sub/subu).
           Though in our emulator, we don't try to catch overflows and C does not complain as well if they occur.
        */
        case OPCODE_ADDIU:
            control.alu_op = FUNCT_ADDU;
            break;
        case OPCODE_SLTI:
            control.alu_op = FUNCT_SLT;
            break;
        case OPCODE_SLTIU:
            control.alu_op = FUNCT_SLTU;
            break;
        case OPCODE_ANDI:
        case OPCODE_ORI:
        case OPCODE_XORI:
        case OPCODE_LUI:
            // doing nothing for these four instructions, notably not setting their alu_op, because they are specifically handled in the alu function.
            // their cases are specified only to prevent them from reaching the default case and accidentally having the reg_write signal turned off.
            break;
        case OPCODE_LB:
        case OPCODE_LH:
        case OPCODE_LW:
        case OPCODE_LBU:
        case OPCODE_LHU:
            control.alu_op = FUNCT_ADD; // for the load instructions, we need the ALU to add the value of $rs to the sign-extended immediate
            control.mem_to_reg = 1; // because the value to be stored in $rt should come from the data memory, not from the ALU result
            control.mem_read = 1; // because we should read data from memory
            break;
        case OPCODE_SB:
        case OPCODE_SH:
        case OPCODE_SW:
            control.alu_op = FUNCT_ADD; // for the same reason as the load instructions
            control.reg_write = 0; // because we are not writing to a register
            control.mem_write = 1; // because we are writing to memory
            break;
        case OPCODE_SPECIAL2:
            // even though mul is not an R-type instruction, it has the exact same format. So we utilize rtype to access the last 6 bits containing the funct
            if (current_instruction.rtype.funct == FUNCT_MUL) {
                control.reg_dest = 1; // we need to write to $rd this time
                control.alu_src = 0; // the second ALU operand should come from $rt
                // not setting alu_op here, because the mul instruction (assuming it's the only one with SPECIAL2 opcode) is specifically handled in the alu function
            }
            break;
        default:
            // most likely an unsupported instruction, so we want to make sure that it doesn't write to a register
            control.reg_write = 0;
            break;
        }
    }
}

// this function sets the branch signal based on the ALU results. It returns 1 if the current instruction is branch-related and thus required some check, or 0 if not
int branch_control(unsigned int opcode, signed long alu_src1, unsigned long alu_result) {
    switch (opcode) {
    case OPCODE_BEQ:
        control.branch = (alu_result == 0) ? 1 : 0;
        break;
    case OPCODE_BNE:
        control.branch = (alu_result != 0) ? 1 : 0;
        break;
    case OPCODE_BLEZ:
        // not relying on the ALU result, but on the value (and sign) of $rs
        control.branch = (alu_src1 <= 0) ? 1 : 0;
        break;
    case OPCODE_BGTZ:
        control.branch = (alu_src1 > 0) ? 1 : 0;
        break;
    default:
        return 0;
        break;
    }

    return 1;
}

void alu(unsigned long src1, unsigned long src2)
{
    long signed_src1 = (long)src1, signed_src2 = (long)src2; // note that long is equivalent to signed long
    long long mult_result; // 64 bits
    unsigned long long multu_result;
    int special = 1; // flag indicating whether we handled one of the special cases (assuming yes at first)

    // handling special cases first
    switch (current_instruction.commontype.opcode) {
    case OPCODE_ANDI:
        alu_result = src1 & (src2 & 0xffffL); // result should contain: src1 & {0 × 16, imm} (L suffix sets the type of the literal to long)
        break;
    case OPCODE_ORI:
        alu_result = src1 | (src2 & 0xffffL); // result should contain: src1 | {0 × 16, imm}
        break;
    case OPCODE_XORI:
        alu_result = src1 ^ (src2 & 0xffffL); // result should contain: src1 ^ {0 × 16, imm}
        break;
    case OPCODE_LUI: // load upper immediate
        alu_result = (src2 << 16) & 0xffff0000L; // result should contain: {(imm)[15:0], 0 × 16}
        break;
    case OPCODE_SPECIAL2: // assuming mul instruction
        /* Multiplying two 32-bit numbers might result in a 64-bit result, from which we need to take the least significant 32 bits according to the
           documentation of mul. Since alu_result is 32 bits in size, this behavior occurs anyway.
        */
        alu_result = signed_src1 * signed_src2;
        break;
    default:
        special = 0; // we are not in the special case
        break;
    }

    // if a special case was already handled, we need to exit the function
    if (special) {
        return;
    }

    switch (control.alu_op) {
    case FUNCT_SLL: // shift left logical (there's no special instruction for shift left arithmetic, because it's essentially the same as left logical)
        alu_result = src2 << current_instruction.rtype.shamt;
        break;
    case FUNCT_SRL: // shift right logical (unsigned right shift)
        alu_result = src2 >> current_instruction.rtype.shamt;
        break;
    case FUNCT_SRA: // shift right arithmetic (signed right shift)
        /* The result of a right-shift of a signed negative number is implementation-dependent, but the Microsoft C++ compiler uses arithmetic shift as needed:
           (https://docs.microsoft.com/en-us/cpp/cpp/left-shift-and-right-shift-operators-input-and-output?view=msvc-170#right-shifts) */
        alu_result = signed_src2 >> current_instruction.rtype.shamt;
        break;
    case FUNCT_SLLV: // shift left logical variable
        alu_result = src2 << src1;
        break;
    case FUNCT_SRLV: // shift right logical variable (unsigned right shift)
        alu_result = src2 >> src1;
        break;
    case FUNCT_SRAV: // shift right arithmetic variable (signed right shift)
        alu_result = signed_src2 >> signed_src1;
        break;
    case FUNCT_MFHI: // move from hi
        alu_result = hi;
        break;
    case FUNCT_MTHI: // move to hi
        hi = src1;
        break;
    case FUNCT_MFLO: // move from lo
        alu_result = lo;
        break;
    case FUNCT_MTLO: // move to lo
        lo = src1;
        break;
    case FUNCT_MULT: // signed multiplication
        mult_result = (long long)signed_src1 * signed_src2; // it is sufficient to cast only one of the operands
        lo = (unsigned long)mult_result; // casting to unsigned long takes only the least significant 32 bits
        hi = (unsigned long)(mult_result >> 32); // shifting the higher 32 bits to the lower part so that they are taken when casting to long
        break;
    case FUNCT_MULTU: // unsigned multiplication
        multu_result = (unsigned long long)src1 * src2;
        lo = (unsigned long)multu_result;
        hi = (unsigned long)(multu_result >> 32);
        break;
    case FUNCT_DIV: // signed division
        lo = signed_src1 / signed_src2;
        hi = signed_src1 % signed_src2;
        break;
    case FUNCT_DIVU: // unsigned division
        lo = src1 / src2;
        hi = src1 % src2;
        break;
    case FUNCT_ADD: // note that all load and store instructions rely on this addition to calculate the memory address from which to load, or to which to store
        alu_result = signed_src1 + signed_src2;
        break;
    case FUNCT_ADDU:
        alu_result = src1 + src2;
        break;
    case FUNCT_SUB:
        alu_result = signed_src1 - signed_src2;
        break;
    case FUNCT_SUBU:
        alu_result = src1 - src2;
        break;
    case FUNCT_AND:
        alu_result = src1 & src2;
        break;
    case FUNCT_OR:
        alu_result = src1 | src2;
        break;
    case FUNCT_XOR:
        alu_result = src1 ^ src2;
        break;
    case FUNCT_NOR: // not or
        alu_result = ~(src1 | src2);
        break;
    case FUNCT_SLT: // set on less than (signed comparison)
        alu_result = (signed_src1 < signed_src2);
        break;
    case FUNCT_SLTU: // set on less than (unsigned comparison)
        alu_result = (src1 < src2);
        break;
    default:
        break;
    }
}

// utility (recursive) function that gets a non-zero number and prints it in binary format
void print_binary(unsigned long num)
{
    if (num == 0) {
        return;
    }
    print_binary(num >> 1);
    printf("%d", num & 1); // printing the LSB
}

// returns whether or not an exit syscall was read
int handle_syscall(void) 
{
    char *str;

    // the syscall code is stored in register $v0 (whose index is given in SYSCALL_CODES_REG)
    switch (registers[SYSCALL_CODES_REG]) {
    case SYSCALL_CODE_PRINT_INT:
        printf("%d", registers[SYSCALL_ARG1_REG]);
        break;
    case SYSCALL_CODE_PRINT_STRING:
        str = (char *)data_mem; // since string is a pointer, it can get the address of the data memory
        str += registers[SYSCALL_ARG1_REG]; // using pointer arithmetic to add the address of the null-terminated string to print, from the register that stores it
        /* Printing characters from this address onward until encountering a null terminator.
           Note: the data file contains the bytes representing string characters in reverse order. Since this computer's CPU architecture is little endian,
           the data read from the file is reversed again when storing inside the data_mem array. So the characters are printed in the correct order.
        */
        printf("%s", str);
        break;
    case SYSCALL_CODE_PRINT_CHAR:
        printf("%c", registers[SYSCALL_ARG1_REG]);
        break;
    case SYSCALL_CODE_PRINT_INT_HEX:
        printf("%x", registers[SYSCALL_ARG1_REG]);
        break;
    case SYSCALL_CODE_PRINT_INT_BIN:
        // if the number is 0, simply printing it
        if (registers[SYSCALL_ARG1_REG] == 0) {
            printf("%d", registers[SYSCALL_ARG1_REG]);
        } else {
            // else use the utility function
            print_binary(registers[SYSCALL_ARG1_REG]);
        }
        break;
    case SYSCALL_CODE_PRINT_UINT:
        printf("%u", registers[SYSCALL_ARG1_REG]);
        break;
    case SYSCALL_CODE_READ_INT:
        scanf("%d", &registers[SYSCALL_CODES_REG]); // the number read should be stored in $v0 (register 2), the same as the syscall codes register
        break;
    case SYSCALL_CODE_SLEEP:
        Sleep(registers[SYSCALL_ARG1_REG]);
        break;
    case SYSCALL_CODE_EXIT:
        printf("\n-- program is finished running --\n");
        return 1; // exiting the step function
        break;
    case SYSCALL_CODE_DRAW_PIXEL:
    case SYSCALL_CODE_DRAW_RECTANGLE:
    case SYSCALL_CODE_DRAW_BITMAP:
        handle_draw_syscalls();
        break;
    default:
        printf("Unknown syscall code %ld\n", registers[SYSCALL_CODES_REG]);
        break;
    }

    return 0;
}

// this function returns the value located at the given address in data memory, based on the size to read, specified by the load instruction (byte/halfword/word)
unsigned long load_from_memory(unsigned long addr)
{
    char *byte_ptr;
    short *hw_ptr; // halfword (16 bits)
    unsigned long result = 0;

    switch (current_instruction.commontype.opcode) {
    case OPCODE_LB:
        byte_ptr = (char *)data_mem; // when dereferencing a char (byte) pointer, we will only get the least significant 8 bits as necessary
        result = byte_ptr[addr % (DATA_MEM_SIZE * 4)]; // we want the address to fall within the data array, but DATA_MEM_SIZE counts words whereas addr counts bytes, so we have to multiply the size by 4
        break;
    case OPCODE_LH:
        hw_ptr = (short *)data_mem; // when dereferencing a short (halfword) pointer, we will only get the least significant 16 bits as necessary
        result = hw_ptr[(addr >> 1) % (DATA_MEM_SIZE * 2)]; // hw_ptr[addr] would evaluate to a memory address 2*addr bytes ahead of hw_ptr, so we have to divide addr by 2
        break;
    case OPCODE_LW:
        // we don't need to use a special pointer since data_mem is already a pointer to 32 bits (word)
        result = data_mem[(addr >> 2) % DATA_MEM_SIZE]; // data_mem[addr] would evaluate to a memory address 4*addr bytes ahead of data_mem, so we have to divide addr by 4
        break;
    case OPCODE_LBU: // load byte unsigned
        byte_ptr = (char *)data_mem;
        result = byte_ptr[addr % (DATA_MEM_SIZE * 4)] & 0xffL; // result should contain: {0 × 24, Mem1B(R[$rs] + SignExt16b(imm))}
        break;
    case OPCODE_LHU: // load halfword unsigned
        hw_ptr = (short *)data_mem;
        result = hw_ptr[(addr >> 1) % (DATA_MEM_SIZE * 2)] & 0xffffL; // result should contain: {0 × 16, Mem2B(R[$rs] + SignExt16b(imm))}
        break;
    default:
        break;
    }

    return result;
}

// this function stores the given value (or part of it) at the given address in data memory, based on the size to store, specified by the store instruction (byte/halfword/word)
void store_in_memory(unsigned long addr, unsigned long value)
{
    char *byte_ptr;
    short *hw_ptr; // halfword (16 bits)

    switch (current_instruction.commontype.opcode) {
    case OPCODE_SB:
        byte_ptr = (char *)data_mem;
        byte_ptr[addr % (DATA_MEM_SIZE * 4)] = (char)value; // taking only the least significant 8 bits from the value
        break;
    case OPCODE_SH:
        hw_ptr = (short *)data_mem;
        hw_ptr[(addr >> 1) % (DATA_MEM_SIZE * 2)] = (short)value; // taking only the least significant 16 bits from the value
        break;
    case OPCODE_SW:
        data_mem[(addr >> 2) % DATA_MEM_SIZE] = value; // storing the entire 32-bit value
        break;
    default:
        break;
    }
}

int MIPS_step(void)
{
    unsigned long alu_src1, alu_src2; // parameters passed to alu
    short imm; // variable to contain the immediate value from the instruction, if it exists (short is 16 bits)
    long sign_ext_imm; // variable to contain the sign-extended immediate value
    unsigned long tmp; // for jr/jalr

    // reading next instruction and setting control signals
    current_instruction.inst = prog_mem[(pc >> 2) % PROG_MEM_SIZE]; // prog_mem contains 32-bit (long) elements, but pc counts bytes, so we divide it by 4 and use modulus so that the index falls in the prog_mem array range
    generate_control();

    alu_src1 = registers[current_instruction.rtype.rs]; // will also work for I-type instructions (whose rs has the same size and position as R-type)

    imm = (short)current_instruction.itype.addr_im; // taking the immediate value from the instruction
    sign_ext_imm = imm; // performing sign extension
    alu_src2 = (control.alu_src) ? sign_ext_imm : registers[current_instruction.rtype.rt]; // choosing the relevant value - rt or the sign-extended immediate (storing sign_ext_imm in an unsigned long does no harm)
    alu(alu_src1, alu_src2); // calculates alu_result

    // checking if we need to write to a register (later on handling jumps writing to R[31])
    if (control.reg_write && !control.jump_and_link) {
        // first checking if we need to read a value from the memory into the register (only load instructions)
        if (control.mem_read && control.mem_to_reg) { // these signals are always both on or both off
            // all load instructions write to $rt
            registers[current_instruction.itype.rt] = load_from_memory(alu_result);
        } else {
            // checking if we need to write to $rd (R-type + mul instruction)
            if (control.reg_dest) {
                registers[current_instruction.rtype.rd] = alu_result; // writing the ALU result
            } else {
                registers[current_instruction.itype.rt] = alu_result; // writing the ALU result to $rt (applies to: addi, addiu, slti, sltiu, andi, ori, xori, lui)
            }
        }
        registers[0] = 0; // making sure no one changed the $zero register
    } else if (control.mem_write) { // checking if we need to write to memory (store instructions)
        store_in_memory(alu_result, registers[current_instruction.itype.rt]);
    } else { // syscalls, jumps and branches
        if (current_instruction.commontype.opcode == OPCODE_RTYPE && current_instruction.rtype.funct == FUNCT_SYSCALL) {
            // handling the syscall and exiting the program if an exit call was made (MARS allows omitting the exit call, but here this is the only way to exit)
            if (handle_syscall() != 0) {
                return 1;
            }
        } else {
            if (control.jump) { // j/jal
                if (control.jump_and_link) {
                    registers[NUM_REG - 1] = pc + 4; // storing the return address in register 31 (also called $ra - return address)
                }
                pc = ((pc + 4) & 0xf0000000) | (current_instruction.jtype.addr << 2); // the new pc should be composed of: {(PC + 4)[31:28], address, 00}
                return 0; // returning here so that we don't add 4 below
            }
            
            if (control.jump_register) { // jr/jalr
                tmp = registers[NUM_REG - 1];
                if (control.jump_and_link) {
                    registers[NUM_REG - 1] = pc + 4;
                }
                pc = tmp;
                return 0;
            }

            // updating branch signal and checking it only if the current instruction is one of the branch instructions
            if (branch_control(current_instruction.commontype.opcode, (long)alu_src1, alu_result)) {
                if (control.branch) {
                    pc += (sign_ext_imm << 2); // we need to add 00 to the low order bits of the sign-extended offset address we should jump to. 4 is also added to the pc afterwards, below
                }
            /* The R-type instructions: mthi, mtlo, mult, multu, div, divu will reach here because they don't match any of the previous cases,
               and in fact we don't need to do anything with them because they already modified hi and lo. But we also need to make sure they are not
               mistaken for unsupported instructions...
            */
            } else if (current_instruction.commontype.opcode != OPCODE_RTYPE) {
                printf("Unsupported instruction: %x\n", current_instruction.inst); // after checking all options, there is nothing left to do...
            }
        }
    }

    pc += 4; // moving on to the next instruction
    
    return 0; // indicating that the program is not finished yet
}
