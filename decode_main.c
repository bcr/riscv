#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "decode.h"

int main(void)
{
    char buffer[128];
    uint32_t instruction;

    while (fgets(buffer, sizeof(buffer), stdin))
    {
        instruction = strtol(buffer, NULL, 16);

        decode_one_instruction(instruction, buffer, sizeof(buffer));
        printf("%08x %s\n", instruction, buffer);
    }
}
