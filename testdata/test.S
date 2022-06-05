start:
    auipc x0,12345
    lui a0, 0x12345
    jal ra, 1
    jalr x2,0x123
#    beq a0,a1,start # Also broken, PC relative
    lb x1,17(x2)
    lh x31,17(x30)
    lw x29,20(x28)
    lbu x27,30(x26)
    lhu x25,40(x24)
    sb x23,9(x22)
    sh x21,41(x20)
    sw x19,133(x18)
    addi x17,x16,57
    slti x15,x14,114
    sltiu x13,x12,228
    xori x11,x10,456
    ori x9,x8,912
    andi x7,x6,1824
    slli x4,x5,30
    srli x3,x2,1
    srai x1,x0,31
    add x31,x30,x29
    sub x28,x27,x26
    sll x25,x24,x23
    slt x22,x21,x20
    sltu x19,x18,x17
    xor x16,x15,x14
    srl x13,x12,x11
    sra x10,x9,x8
    or x7,x6,x5
    and x4,x3,x2
    # Fence instructions are i,o,r,w for pred and succ, probably need to test
    fence i,r
    fence io,rw
    fence iorw,i
    fence iw,or
    fence
    fence.i
    ecall
    ebreak
    # CSR functions have a bunch of special names
    csrrw x1,cycle,x0
    csrrs x31,instreth,x30
    csrrc x29,cycleh,x28
    csrrwi x27,instret,31
    csrrsi x26,timeh,30
    csrrci x25,fcsr,29

    # pseudo instructions
    nop
    ret
