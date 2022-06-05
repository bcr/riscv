#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "decode.h"
#include "decode_handlers.h"

typedef bool (*decode_func_t)(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);

struct instruction_entry
{
    uint32_t mask;
    uint32_t match;
    const char* opcode;
    decode_func_t handler;
};

#define OPCODE_MASK 0x7f
#define FUNCT3_MASK 0x7000
#define FUNCT7_MASK 0xFE000000
#define FENCE_MASK  0xF00FFFFF
#define EXACT_MASK  0xFFFFFFFF

static const struct instruction_entry instructions[] = {
    /* Psudoinstructions */
    { .mask = EXACT_MASK, .match = 0x13, .opcode = "nop", .handler = decode_opcode_only },
    { .mask = EXACT_MASK, .match = 0x8067, .opcode = "ret", .handler = decode_opcode_only },

    /* RV32I */
    { .mask = OPCODE_MASK, .match = 0x37, .opcode = "lui", .handler = decode_u },
    { .mask = OPCODE_MASK, .match = 0x17, .opcode = "auipc", .handler = decode_u },
    { .mask = OPCODE_MASK, .match = 0x6f, .opcode = "jal", .handler = decode_j },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x67, .opcode = "jalr", .handler = decode_i },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x63, .opcode = "beq", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1063, .opcode = "bne", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x4063, .opcode = "blt", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x5063, .opcode = "bge", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x6063, .opcode = "bltu", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x7063, .opcode = "bgeu", .handler = decode_b },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3, .opcode = "lb", .handler = decode_i_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1003, .opcode = "lh", .handler = decode_i_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2003, .opcode = "lw", .handler = decode_i_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x4003, .opcode = "lbu", .handler = decode_i_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x5003, .opcode = "lhu", .handler = decode_i_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x23, .opcode = "sb", .handler = decode_s },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1023, .opcode = "sh", .handler = decode_s },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2023, .opcode = "sw", .handler = decode_s },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x13, .opcode = "addi", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2013, .opcode = "slti", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3013, .opcode = "sltiu", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x4013, .opcode = "xori", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x6013, .opcode = "ori", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x7013, .opcode = "andi", .handler = decode_i_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x1013, .opcode = "slli", .handler = decode_i_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x5013, .opcode = "srli", .handler = decode_i_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40005013, .opcode = "srai", .handler = decode_i_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x33, .opcode = "add", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40000033, .opcode = "sub", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x1033, .opcode = "sll", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x2033, .opcode = "slt", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x3033, .opcode = "sltu", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x4033, .opcode = "xor", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x5033, .opcode = "srl", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40005033, .opcode = "sra", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x6033, .opcode = "or", .handler = decode_r },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x7033, .opcode = "and", .handler = decode_r },
    { .mask = FENCE_MASK, .match = 0x0F, .opcode = "fence", .handler = decode_fence },
    { .mask = EXACT_MASK, .match = 0x100F, .opcode = "fence.i", .handler = decode_opcode_only },
    { .mask = EXACT_MASK, .match = 0x73, .opcode = "ecall", .handler = decode_opcode_only },
    { .mask = EXACT_MASK, .match = 0x100073, .opcode = "ebreak", .handler = decode_opcode_only },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1073, .opcode = "csrrw", .handler = decode_csr },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2073, .opcode = "csrrs", .handler = decode_csr },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3073, .opcode = "csrrc", .handler = decode_csr },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x5073, .opcode = "csrrwi", .handler = decode_csr_imm },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x6073, .opcode = "csrrsi", .handler = decode_csr_imm },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x7073, .opcode = "csrrci", .handler = decode_csr_imm },

    { .mask = 0 }
};

size_t decode_one_instruction(uint32_t pc, uint32_t instruction, char* output, size_t output_length)
{
    bool handled = false;

    output[0] = '\0';

    for (const struct instruction_entry* mover = instructions; mover->mask; ++mover)
    {
        if ((instruction & mover->mask) == mover->match)
        {
            handled = mover->handler(pc, instruction, mover->opcode, output, output_length);
            if (handled)
            {
                break;
            }
        }
    }

    if (!handled)
    {
        snprintf(output, output_length, "%s", "????");
    }

    return 0;
}
