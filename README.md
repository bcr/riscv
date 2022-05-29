# RISC-V Explorations

## Observations

I think the general strategy I am trying to follow is to use the RISC-V `as`
assembler to generate output, and then use `objdump` to get the expected
output, then trying to match that.

### Encoding and Decoding Fun

In the following example, I used the `la` pseudoinstruction that could not be
directly reconciled at disassembly time -- you can see it understands the
effective address based on the comment for the `addi` instruction, but it
stopped short of decoding it as `la`.

```asm
    la a0,0x12345

   0:   00012537                lui     a0,0x12
   4:   34550513                addi    a0,a0,837 # 0x12345
```

However, in the case of the `nop` pseudoinstruction, it is generated as
`addi x0,x0,0` which always decodes as `nop`.

```asm
    nop
    addi x0,x0,0

   0:   00000013                nop
   4:   00000013                nop
```

Interestingly, other `addi` operations with `rd` as the `x0` register decode as
`mv` pseudoinstructions.

```asm
    addi x0,x0,0
    addi x0,x1,0
    addi x1,x2,0

   0:   00000013                nop
   4:   00008013                mv      zero,ra
   8:   00010093                mv      ra,sp
```

## Useful Links

<https://stackoverflow.com/questions/1484817/how-do-i-make-a-simple-makefile-for-gcc-on-linux>
<https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf>
