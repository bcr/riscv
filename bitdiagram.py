input = """
31 0000000
24 rs2
19 rs1
14 000
11 rd
6 0110011
"""

fields = []

last_split = None

for line in input.splitlines():
    if not line:
        continue
    splitline = line.split()
    splitline[0] = int(splitline[0])
    if last_split:
        last_split.append(last_split[0] - splitline[0])
    fields.append(splitline)
    last_split = splitline

last_split.append(last_split[0] - -1)

output = "LaTeX"

if output == "ASCII":
    # ASCII output
    spaces_per_bit = 2

    for field in fields:
        value = str(field[0])
        print(value, end='')
        print((field[2] * spaces_per_bit - len(value)) * ' ', end='')

    print()

    for field in fields:
        print('+', end='')
        print('-' * ((field[2] * spaces_per_bit) - 1), end='')

    print('+')

    for field in fields:
        print(f"|{field[1]:^{(field[2] * spaces_per_bit) - 1}}", end='')
        # print(f"{:^{field[1]}{field[2]}}")
    print('|')

    for field in fields:
        print('+', end='')
        print('-' * ((field[2] * spaces_per_bit) - 1), end='')

    print('+')
else:
    #LaTeX output
    print('\\begin{tabular}{', end='')

    max_bit = None
    for field in fields:
        max_bit = max_bit or field[0]
        print(f'p{{{field[2] / max_bit * 0.9}\\textwidth}}', end='')
    print('}')

    first_field = True
    for field in fields:
        if not first_field:
            print('&', end='')
        else:
            first_field = False
        print(f'\\scriptsize{{{field[0]}}}', end='')
        # print(f'\\multicolumn{{1}}{{|l|}}{{{field[0]}}}', end='')
    print('\\\\\\hline')

    first_field = True
    for field in fields:
        if not first_field:
            print('&', end='')
        else:
            first_field = False
        # print(field[1], end='')
        print(f'\\multicolumn{{1}}{{|c|}}{{\\texttt{{{field[1]}}}}}', end='')
    print('\\\\\\hline')

    print('\\end{tabular}')
