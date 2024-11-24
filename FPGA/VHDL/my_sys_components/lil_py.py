
import os


if __name__ == '__main__':

    s = ''
    for i in range(200):
        s += f'{i:02x} 0000\n'

    with open('ram_content.txt', 'w') as f:
        f.write(s)
    