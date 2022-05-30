start:
    auipc x0,12345
    lui a0, 0x12345
#    jal ra, 1  # Not working yet, PC relative shenanigans
    jalr x2,0x123
#    beq a0,a1,start # Also broken, PC relative
    lb x1,17(x2)
    lh x31,17(x30)
    lw x29,20(x28)
    lbu x27,30(x26)
    lhu x25,40(x24)
