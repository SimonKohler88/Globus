import os
import shutil

if __name__ == '__main__':
    p = 'acq0001.csv'
    img = None

    if os.path.exists('splitted'):
        shutil.rmtree('splitted')
    os.mkdir('splitted')

    for each in os.listdir('.'):
        if each.endswith('.csv'):
            with open(each, 'r') as f:
                lines = f.readlines()
            print(each)
            # print(len(lines))
            ln = lines[8:]
            lnSPI = [l for l in ln if 'SPI,' in l]
            lnSPI2 = [l for l in ln if 'SPI2,' in l]

            nm, ext = os.path.splitext(each)
            new_nm1 = f'{nm}_SPI'
            new_nm2 = f'{nm}_SPI2'

            with open(f'splitted/{new_nm1}{ext}', 'w') as f:
                f.write(''.join(lnSPI))

            with open(f'splitted/{new_nm2}{ext}', 'w') as f:
                f.write(''.join(lnSPI2))