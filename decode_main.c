#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "decode.h"

int main(void)
{
    char output[128];
    decode_one_instruction(0x3c025537, output, sizeof(output));
    printf("%s\n", output);
}
