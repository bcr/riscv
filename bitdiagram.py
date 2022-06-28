import shutil
import tempfile

def modfile(filename: str):
    # Make a temporary file for text read/write
    temporary_file = tempfile.TemporaryFile('w+')
    # Open the original file for text read/write
    original_file = open(filename, 'r+')
    # Copy all the contents of the original file to the temporary file
    shutil.copyfileobj(original_file, temporary_file)
    # Rewind the temporary file, this is the "input"
    temporary_file.seek(0)
    input_file = temporary_file
    # Clear the original file, this is the "output"
    original_file.seek(0)
    original_file.truncate(0)
    output_file = original_file
    return (input_file, output_file)

(input_file, output_file) = modfile('test.input')
with input_file, output_file:
    # Copy lines until our section
    for line in input_file:
        print(line, end='', file=output_file)
        if line.rstrip() == '\\subsubsection{Encoding}':
            break

    for line in input_file:
        print(line, end='', file=output_file)
        if line.rstrip() == '\\iffalse':
            break


    fields = []

    last_split = None

    # Parse our input until the end of the bit fields
    for line in input_file:
        print(line, end='', file=output_file)
        line = line.rstrip()
        if line == '\\fi':
            break
        splitline = line.split()
        splitline[0] = int(splitline[0])
        if last_split:
            last_split.append(last_split[0] - splitline[0])
        fields.append(splitline)
        last_split = splitline

    last_split.append(last_split[0] - -1)

    #LaTeX output
    print('\\begin{tabular}{', end='', file=output_file)

    max_bit = None
    for field in fields:
        max_bit = max_bit or field[0]
        print(f'p{{{field[2] / max_bit * 0.9}\\textwidth}}', end='', file=output_file)
    print('}', file=output_file)

    first_field = True
    for field in fields:
        if not first_field:
            print('&', end='', file=output_file)
        else:
            first_field = False
        print(f'\\scriptsize{{{field[0]}}}', end='', file=output_file)
        # print(f'\\multicolumn{{1}}{{|l|}}{{{field[0]}}}', end='')
    print('\\\\\\hline', file=output_file)

    first_field = True
    for field in fields:
        if not first_field:
            print('&', end='', file=output_file)
        else:
            first_field = False
        # print(field[1], end='')
        print(f'\\multicolumn{{1}}{{|c|}}{{\\texttt{{{field[1]}}}}}', end='', file=output_file)
    print('\\\\\\hline', file=output_file)

    print('\\end{tabular}', file=output_file)

    # Skip old stuff from original file
    for line in input_file:
        line = line.rstrip()
        if line == '\\end{tabular}':
            break

    # Copy over any remainder
    shutil.copyfileobj(input_file, output_file)
