#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "decode.h"

int main(void)
{
    char buffer[128];
    uint32_t address;
    uint32_t instruction;
    const char* instruction_ptr;

    while (fgets(buffer, sizeof(buffer), stdin))
    {
        address = strtol(buffer, NULL, 16);
        instruction_ptr = strchr(buffer, ':');
        if (!instruction_ptr)
        {
            continue;
        }
        instruction = strtol(instruction_ptr + 1, NULL, 16);

        decode_one_instruction(instruction, buffer, sizeof(buffer));
        printf("%4x:\t%08x %s\n", address, instruction, buffer);
    }
}
