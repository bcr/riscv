#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "decode.h"

enum instruction_type
{
    I_U,
    I_J,
    I_R,
    I_I,
    I_S,
    I_B
};

struct instruction_entry
{
    uint32_t mask;
    uint32_t match;
    const char* opcode;
    enum instruction_type instruction_type;
};

#define OPCODE_MASK 0x7f
#define FUNCT3_MASK 0x3000

static const struct instruction_entry instructions[] = {
    { .mask = OPCODE_MASK, .match = 0x37, .opcode = "lui", .instruction_type = I_U },
    { .mask = OPCODE_MASK, .match = 0x17, .opcode = "auipc", .instruction_type = I_U },
    { .mask = OPCODE_MASK, .match = 0x6f, .opcode = "jal", .instruction_type = I_J },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x67, .opcode = "jalr", .instruction_type = I_I },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x63, .opcode = "beq", .instruction_type = I_B },

    { .mask = 0 }
};

// 3c025537                lui     a0,0x3c025

static const char* (abi_register_names[32]) = {
    "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2", // x0  - x7
    "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5", // x8  - x15
    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7", // x16 - x23
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"  // x24 - x31
};

// 01010 0110111

#define extract_rd() rd = (instruction >> 7) & 0x1F
#define extract_rs1() rs1 = (instruction >> 15) & 0x1F
#define extract_rs2() rs2 = (instruction >> 20) & 0x1F
#define extract_U_imm() imm = (instruction & 0xFFFFF000) >> 12
#define extract_J_imm() imm = (instruction & 0x000FF000) | ((instruction & 0x00100000) >> 9) | ((instruction & 0x7FE00000) >> 20) | ((instruction & 0x80000000) >> 11)
#define extract_I_imm() imm = (instruction & 0xFFF00000) >> 20
#define extract_B_imm() imm = (instruction & 0x0F00) >> 7 | ((instruction & 0x7E000000) >> 20) | ((instruction & 0x080) << 4) | ((instruction & 0x80000000) >> 19) | ((instruction & 0x80000000) ? 0xFFFFF000 : 0)

size_t decode_one_instruction(uint32_t instruction, char* output, size_t output_length)
{
    bool handled = false;

    output[0] = '\0';

    for (const struct instruction_entry* mover = instructions; mover->mask; ++mover)
    {
        if ((instruction & mover->mask) == mover->match)
        {
            switch (mover->instruction_type)
            {
                uint8_t rd;
                uint8_t rs1;
                uint8_t rs2;
                int32_t imm;
                case I_U:
                    extract_rd();
                    extract_U_imm();
                    snprintf(output, output_length, "%s\t%s,%#x", mover->opcode, abi_register_names[rd], imm);
                    break;

                case I_J:
                    extract_rd();
                    extract_J_imm();
                    snprintf(output, output_length, "%s\t%s,%#x", mover->opcode, abi_register_names[rd], imm);
                    break;

                case I_I:
                    extract_rs1();
                    extract_I_imm();
                    snprintf(output, output_length, "%s\t%d(%s)", mover->opcode, imm, abi_register_names[rs1]);
                    break;

                case I_B:
                    extract_rs1();
                    extract_rs2();
                    extract_B_imm();
                    snprintf(output, output_length, "%s\t%s,%s,%d", mover->opcode, abi_register_names[rs1], abi_register_names[rs2], imm);
                    break;

                default:
                    break;
            }
            handled = true;
            break;
        }
    }

    if (!handled)
    {
        snprintf(output, output_length, "%s", "????");
    }

    return 0;
}
