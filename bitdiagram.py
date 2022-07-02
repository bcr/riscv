import fileinput
import re

def output_latex(fields):
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

with fileinput.input(files=('test.input'), inplace=True) as input_file:
    # Copy lines until our section
    for line in input_file:
        print(line, end='')
        if re.search("Begin encoding definition", line):
            break

    fields = []

    last_split = None

    # Parse our input until the end of the bit fields
    for line in input_file:
        print(line, end='')
        line = line.rstrip()
        if re.search("End encoding definition", line):
            break
        splitline = line.split()
        splitline[0] = int(splitline[0])
        if last_split:
            last_split.append(last_split[0] - splitline[0])
        fields.append(splitline)
        last_split = splitline

    last_split.append(last_split[0] - -1)

    # Copy lines until our output section
    for line in input_file:
        print(line, end='')
        if re.search("Begin encoding output", line):
            break

    output_latex(fields)

    # Skip old stuff from original file
    for line in input_file:
        if re.search("End encoding output", line):
            print(line, end='')
            break

    # Copy over any remainder
    for line in input_file:
        print(line, end='')
