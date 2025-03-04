

f = 'Earth_relief_120x256_raw2_inverted'

with open(f'{f}.txt', 'r') as fily:
    all_lines = fily.readlines()
cols = 8
rows = 120
new_name = f'{f}_reshaped_{cols}x{rows}.txt'
with open(new_name, 'w') as fily:
    ln = ' ' * 6 + ''.join([f'{i:>10}' for i in range(cols)]) +'\n'
    fily.write(ln)

new_lines_ordered = []
for row in range(len(all_lines) // cols):
    # print(row)
    ln = f'{row:<6}' + ''.join([f'{d.strip():>10}' for d in all_lines[row *cols : row *cols+cols ]]) + '\n'
    new_lines_ordered.append(ln)

# print(new_lines_ordered)



with open(new_name, 'a') as fily:
    for each in new_lines_ordered:
        fily.write(each)