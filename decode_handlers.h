#ifndef DECODE_HANDLERS_H
#define DECODE_HANDLERS_H

bool decode_u(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_j(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_i(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_i_2(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_i_3(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_i_4(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_b(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_s(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_r(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_fence(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_opcode_only(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_csr(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);
bool decode_csr_imm(uint32_t pc, uint32_t instruction, const char* opcode, char* output, size_t output_length);

#endif /* DECODE_HANDLERS_H */
