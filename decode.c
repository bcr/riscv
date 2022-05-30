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

static const struct instruction_entry instructions[] = {
    { .mask = OPCODE_MASK, .match = 0x37, .opcode = "lui", .instruction_type = I_U },
    { .mask = OPCODE_MASK, .match = 0x17, .opcode = "auipc", .instruction_type = I_U },

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
                uint32_t imm;
                case I_U:
                    extract_rd();
                    extract_U_imm();
                    snprintf(output, output_length, "%s\t%s,%#x", mover->opcode, abi_register_names[rd], imm);
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
