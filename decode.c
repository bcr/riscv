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
    I_I_2,
    I_I_3,
    I_I_4,
    I_S,
    I_B,
    I_FENCE,
    I_OPCODE_ONLY,
    I_CSR,
    I_CSR_IMM,
};

struct instruction_entry
{
    uint32_t mask;
    uint32_t match;
    const char* opcode;
    enum instruction_type instruction_type;
};

struct csr_entry
{
    uint16_t number;
    const char* name;
};

// From Table 19.3 in RISC-V User-Level ISA V2.2
static const struct csr_entry csr_entries[] = {
    { .number = 0x0001, .name = "fflags" },
    { .number = 0x0002, .name = "frm" },
    { .number = 0x0003, .name = "fcsr" },
    { .number = 0x0C00, .name = "cycle" },
    { .number = 0x0C02, .name = "instret" },
    { .number = 0x0C80, .name = "cycleh" },
    { .number = 0x0C81, .name = "timeh" },
    { .number = 0x0C82, .name = "instreth" },

    { .number = 0, .name = "????" }
};

#define OPCODE_MASK 0x7f
#define FUNCT3_MASK 0x7000
#define FUNCT7_MASK 0xFE000000
#define FENCE_MASK  0xF00FFFFF
#define EXACT_MASK  0xFFFFFFFF

static const struct instruction_entry instructions[] = {
    { .mask = EXACT_MASK, .match = 0x13, .opcode = "nop", .instruction_type = I_OPCODE_ONLY },
    { .mask = EXACT_MASK, .match = 0x8067, .opcode = "ret", .instruction_type = I_OPCODE_ONLY },

    { .mask = OPCODE_MASK, .match = 0x37, .opcode = "lui", .instruction_type = I_U },
    { .mask = OPCODE_MASK, .match = 0x17, .opcode = "auipc", .instruction_type = I_U },
    { .mask = OPCODE_MASK, .match = 0x6f, .opcode = "jal", .instruction_type = I_J },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x67, .opcode = "jalr", .instruction_type = I_I },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x63, .opcode = "beq", .instruction_type = I_B },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3, .opcode = "lb", .instruction_type = I_I_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1003, .opcode = "lh", .instruction_type = I_I_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2003, .opcode = "lw", .instruction_type = I_I_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x4003, .opcode = "lbu", .instruction_type = I_I_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x5003, .opcode = "lhu", .instruction_type = I_I_2 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x23, .opcode = "sb", .instruction_type = I_S },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1023, .opcode = "sh", .instruction_type = I_S },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2023, .opcode = "sw", .instruction_type = I_S },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x13, .opcode = "addi", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2013, .opcode = "slti", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3013, .opcode = "sltiu", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x4013, .opcode = "xori", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x6013, .opcode = "ori", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x7013, .opcode = "andi", .instruction_type = I_I_3 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x1013, .opcode = "slli", .instruction_type = I_I_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x5013, .opcode = "srli", .instruction_type = I_I_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40005013, .opcode = "srai", .instruction_type = I_I_4 },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x33, .opcode = "add", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40000033, .opcode = "sub", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x1033, .opcode = "sll", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x2033, .opcode = "slt", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x3033, .opcode = "sltu", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x4033, .opcode = "xor", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x5033, .opcode = "srl", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x40005033, .opcode = "sra", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x6033, .opcode = "or", .instruction_type = I_R },
    { .mask = OPCODE_MASK | FUNCT3_MASK | FUNCT7_MASK, .match = 0x7033, .opcode = "and", .instruction_type = I_R },
    { .mask = FENCE_MASK, .match = 0x0F, .opcode = "fence", .instruction_type = I_FENCE },
    { .mask = EXACT_MASK, .match = 0x100F, .opcode = "fence.i", .instruction_type = I_OPCODE_ONLY },
    { .mask = EXACT_MASK, .match = 0x73, .opcode = "ecall", .instruction_type = I_OPCODE_ONLY },
    { .mask = EXACT_MASK, .match = 0x100073, .opcode = "ebreak", .instruction_type = I_OPCODE_ONLY },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x1073, .opcode = "csrrw", .instruction_type = I_CSR },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x2073, .opcode = "csrrs", .instruction_type = I_CSR },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x3073, .opcode = "csrrc", .instruction_type = I_CSR },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x5073, .opcode = "csrrwi", .instruction_type = I_CSR_IMM },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x6073, .opcode = "csrrsi", .instruction_type = I_CSR_IMM },
    { .mask = OPCODE_MASK | FUNCT3_MASK, .match = 0x7073, .opcode = "csrrci", .instruction_type = I_CSR_IMM },

    { .mask = 0 }
};

static const char* (abi_register_names[32]) = {
    "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2", // x0  - x7
    "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5", // x8  - x15
    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7", // x16 - x23
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"  // x24 - x31
};

// i = 8, o = 4, r = 2, w = 1
static const char* (fence_flags[16]) = {
    "", "w", "r", "rw", "o", "ow", "or", "orw",
    "i", "iw", "ir", "irw", "io", "iow", "ior", "iorw"
};

#define extract_rd() rd = (instruction >> 7) & 0x1F
#define extract_rs1() rs1 = (instruction >> 15) & 0x1F
#define extract_rs2() rs2 = (instruction >> 20) & 0x1F
#define extract_shamt() shamt = (instruction >> 20) & 0x1F
#define extract_pred() pred = (instruction >> 24) & 0x0F
#define extract_succ() succ = (instruction >> 20) & 0x0F
#define extract_csr() csr = (instruction >> 20) & 0x0FFF
#define extract_U_imm() imm = (instruction & 0xFFFFF000) >> 12
#define extract_J_imm() imm = (instruction & 0x000FF000) | ((instruction & 0x00100000) >> 9) | ((instruction & 0x7FE00000) >> 20) | ((instruction & 0x80000000) >> 11)
#define extract_I_imm() imm = (instruction & 0xFFF00000) >> 20
#define extract_B_imm() imm = (instruction & 0x0F00) >> 7 | ((instruction & 0x7E000000) >> 20) | ((instruction & 0x080) << 4) | ((instruction & 0x80000000) >> 19) | ((instruction & 0x80000000) ? 0xFFFFF000 : 0)
#define extract_S_imm() imm = ((instruction & 0x0F80) >> 7) | ((instruction & 0xFE000000) >> 20)
#define extract_CSR_imm() imm = (instruction >> 15) & 0x1F

static const char* csr_to_name(uint16_t csr)
{
    const char* final_string = "????";

    for (const struct csr_entry* mover = csr_entries; mover->number; ++mover)
    {
        if (mover->number == csr)
        {
            final_string = mover->name;
        }
    }

    return final_string;
}

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
                uint8_t shamt;
                uint8_t pred;
                uint8_t succ;
                uint16_t csr;
                int32_t imm;
                const char* csr_name;

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

                case I_I_2:
                    extract_rd();
                    extract_rs1();
                    extract_I_imm();
                    snprintf(output, output_length, "%s\t%s,%d(%s)", mover->opcode, abi_register_names[rd], imm, abi_register_names[rs1]);
                    break;

                case I_I_3:
                    extract_rd();
                    extract_rs1();
                    extract_I_imm();
                    snprintf(output, output_length, "%s\t%s,%s,%d", mover->opcode, abi_register_names[rd], abi_register_names[rs1], imm);
                    break;

                case I_I_4:
                    extract_rd();
                    extract_rs1();
                    extract_shamt();
                    snprintf(output, output_length, "%s\t%s,%s,%#x", mover->opcode, abi_register_names[rd], abi_register_names[rs1], shamt);
                    break;

                case I_B:
                    extract_rs1();
                    extract_rs2();
                    extract_B_imm();
                    snprintf(output, output_length, "%s\t%s,%s,%d", mover->opcode, abi_register_names[rs1], abi_register_names[rs2], imm);
                    break;

                case I_S:
                    extract_rs1();
                    extract_rs2();
                    extract_S_imm();
                    snprintf(output, output_length, "%s\t%s,%d(%s)", mover->opcode, abi_register_names[rs2], imm, abi_register_names[rs1]);
                    break;

                case I_R:
                    extract_rd();
                    extract_rs1();
                    extract_rs2();
                    snprintf(output, output_length, "%s\t%s,%s,%s", mover->opcode, abi_register_names[rd], abi_register_names[rs1], abi_register_names[rs2]);
                    break;

                case I_FENCE:
                    extract_pred();
                    extract_succ();
                    if ((pred == 0x0f) && (succ == 0x0f))
                    {
                        snprintf(output, output_length, "%s", mover->opcode);
                    }
                    else
                    {
                        snprintf(output, output_length, "%s\t%s,%s", mover->opcode, fence_flags[pred], fence_flags[succ]);
                    }
                    break;

                case I_OPCODE_ONLY:
                    snprintf(output, output_length, "%s", mover->opcode);
                    break;

                case I_CSR:
                    extract_rd();
                    extract_rs1();
                    extract_csr();
                    csr_name = csr_to_name(csr);
                    snprintf(output, output_length, "%s\t%s,%s,%s", mover->opcode, abi_register_names[rd], csr_name, abi_register_names[rs1]);
                    break;

                case I_CSR_IMM:
                    extract_rd();
                    extract_CSR_imm();
                    extract_csr();
                    csr_name = csr_to_name(csr);
                    snprintf(output, output_length, "%s\t%s,%s,%d", mover->opcode, abi_register_names[rd], csr_name, imm);
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
