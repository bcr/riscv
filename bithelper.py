input = """
opcode
6 6-0

rd
11 4-0

funct3
14 2-0

rs1
19 4-0

rs2
24 4-0

funct7
31 6-0

i_imm
31 11-0

s_imm
31 11-5
11 4-0

b_imm
31 12 10-5
11 4-1 11

u_imm
31 19-0

j_imm
31 20 10-1 11 19-12

shamt
24 4-0

pred
27 3-0

succ
23 3-0

csr
31 11-0

zimm
19 4-0
"""

first = True

max_destination = 0
final_dict = {}
current_dict = {}

for line in input.splitlines():
    if not line:
        continue
    if not line[0].isdigit():
        current_dict["leftmost_bit_position"] = max_destination + 1
        current_dict = {}
        final_dict[line] = current_dict
        current_dict["spans"] = []
        first = True
        continue
    values = line.split()
    source = int(values[0])
    for destination in values[1:]:
        if first:
            max_destination = 0
            first = False

        if destination.find('-') != -1:
            (high,low) = destination.split('-')
        else:
            high = destination
            low = destination

        high = int(high)
        max_destination = max(max_destination, high)
        low = int(low)

        current_dict["spans"].append((source, high, low))
        source -= (high - low + 1)

current_dict["leftmost_bit_position"] = max_destination + 1

for entry in final_dict:
    print(f"{entry} = ", end='')
    first = True
    for (source, high, low) in final_dict[entry]["spans"]:
        if first:
            first = False
        else:
            print(' | ', end='')

        if source > high:
            print(f'((input >> {source - high})', end='')
        elif source < high:
            print(f'((input << {high - source})', end='')
        else:
            print(f'(input', end='')

        mask = int('1' * (high - low + 1), 2) << low
        mask = hex(mask)
        mask = mask.replace('0x', '0x0')
        print(f' & {mask})', end='')

    print(f" /* {final_dict[entry]['leftmost_bit_position']} */")
