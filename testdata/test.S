start:
    auipc x0,12345
    lui a0, 0x12345
#    jal ra, 1  # Not working yet, PC relative shenanigans
    jalr x2,0x123
#    beq a0,a1,start # Also broken, PC relative
