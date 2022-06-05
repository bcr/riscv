#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "decode_handlers.h"

#define extract_rd() uint8_t rd = (instruction >> 7) & 0x1F
#define extract_rs1() uint8_t rs1 = (instruction >> 15) & 0x1F
#define extract_rs2() uint8_t rs2 = (instruction >> 20) & 0x1F
#define extract_shamt() uint8_t shamt = (instruction >> 20) & 0x1F
#define extract_pred() uint8_t pred = (instruction >> 24) & 0x0F
#define extract_succ() uint8_t succ = (instruction >> 20) & 0x0F
#define extract_csr() uint16_t csr = (instruction >> 20) & 0x0FFF
#define extract_U_imm() int32_t imm = (instruction & 0xFFFFF000) >> 12
#define extract_J_imm() int32_t imm = (instruction & 0x000FF000) | ((instruction & 0x00100000) >> 9) | ((instruction & 0x7FE00000) >> 20) | ((instruction & 0x80000000) >> 11)
#define extract_decode_imm() int32_t imm = (instruction & 0xFFF00000) >> 20
#define extract_B_imm() int32_t imm = (instruction & 0x0F00) >> 7 | ((instruction & 0x7E000000) >> 20) | ((instruction & 0x080) << 4) | ((instruction & 0x80000000) >> 19) | ((instruction & 0x80000000) ? 0xFFFFF000 : 0)
#define extract_S_imm() int32_t imm = ((instruction & 0x0F80) >> 7) | ((instruction & 0xFE000000) >> 20)
#define extract_CSR_imm() int32_t imm = (instruction >> 15) & 0x1F

static const char* (abi_register_names[32]) = {
    "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2", // x0  - x7
    "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5", // x8  - x15
    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7", // x16 - x23
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"  // x24 - x31
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

// i = 8, o = 4, r = 2, w = 1
static const char* (fence_flags[16]) = {
    "", "w", "r", "rw", "o", "ow", "or", "orw",
    "i", "iw", "ir", "irw", "io", "iow", "ior", "iorw"
};

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

#if 0
static int32_t sign_extend(int32_t number, unsigned int bit)
{
    int32_t return_number = number;

    if (number & (1 << bit))
    {
        return_number |= 0xFFFFFFFF << bit;
    }

    return return_number;
}
#endif /* 0 */

bool decode_u(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_U_imm();

    snprintf(output, output_length, "%s\t%s,%#x", opcode, abi_register_names[rd], imm);

    return true;
}

bool decode_j(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_J_imm();

    snprintf(output, output_length, "%s\t%s,%d", opcode, abi_register_names[rd], pc + imm);

    return true;
}

bool decode_i(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rs1();
    extract_decode_imm();

    snprintf(output, output_length, "%s\t%d(%s)", opcode, imm, abi_register_names[rs1]);

    return true;
}

bool decode_i_2(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_rs1();
    extract_decode_imm();

    snprintf(output, output_length, "%s\t%s,%d(%s)", opcode, abi_register_names[rd], imm, abi_register_names[rs1]);

    return true;
}

bool decode_i_3(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_rs1();
    extract_decode_imm();

    snprintf(output, output_length, "%s\t%s,%s,%d", opcode, abi_register_names[rd], abi_register_names[rs1], imm);

    return true;
}

bool decode_i_4(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_rs1();
    extract_shamt();

    snprintf(output, output_length, "%s\t%s,%s,%#x", opcode, abi_register_names[rd], abi_register_names[rs1], shamt);

    return true;
}

bool decode_b(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rs1();
    extract_rs2();
    extract_B_imm();

    snprintf(output, output_length, "%s\t%s,%s,%d", opcode, abi_register_names[rs1], abi_register_names[rs2], pc + imm);

    return true;
}

bool decode_s(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rs1();
    extract_rs2();
    extract_S_imm();

    snprintf(output, output_length, "%s\t%s,%d(%s)", opcode, abi_register_names[rs2], imm, abi_register_names[rs1]);

    return true;
}

bool decode_r(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_rs1();
    extract_rs2();

    snprintf(output, output_length, "%s\t%s,%s,%s", opcode, abi_register_names[rd], abi_register_names[rs1], abi_register_names[rs2]);

    return true;
}

bool decode_fence(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_pred();
    extract_succ();

    if ((pred == 0x0f) && (succ == 0x0f))
    {
        snprintf(output, output_length, "%s", opcode);
    }
    else
    {
        snprintf(output, output_length, "%s\t%s,%s", opcode, fence_flags[pred], fence_flags[succ]);
    }
    return true;
}

bool decode_opcode_only(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    snprintf(output, output_length, "%s", opcode);

    return true;
}

bool decode_csr(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_rs1();
    extract_csr();
    const char* csr_name = csr_to_name(csr);

    snprintf(output, output_length, "%s\t%s,%s,%s", opcode, abi_register_names[rd], csr_name, abi_register_names[rs1]);

    return true;
}

bool decode_csr_imm(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length)
{
    extract_rd();
    extract_CSR_imm();
    extract_csr();
    const char* csr_name = csr_to_name(csr);

    snprintf(output, output_length, "%s\t%s,%s,%d", opcode, abi_register_names[rd], csr_name, imm);

    return true;
}
