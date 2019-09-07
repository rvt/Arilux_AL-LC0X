# Modified from https://jared.geek.nz/2013/feb/linear-led-pwm
# L* = 903.3 * Y,            if Y <= 0.008856
# L* = 116   * Y^1/3 - 16,   if Y >= 0.008856

INPUT_SIZE = 200.0       # Input integer size
OUTPUT_SIZE = 100.0      # Output integer size
INT_TYPE = 'const float'
PROGMEM = 'PROGMEM'
TABLE_NAME = 'cie1931';

def cie1931(L):
    L = L*100.0
    if L <= 8:
        return (L/902.3)
    else:
        return ((L+16.0)/116.0)**3

x = range(0,int(INPUT_SIZE+1))
y = [(cie1931(float(L)/INPUT_SIZE)*OUTPUT_SIZE) for L in x]

with open('cie1931.h', 'w') as f:
    f.write('// CIE1931 correction table\n')
    f.write('// Automatically generated\n\n')
    f.write('// See https://jared.geek.nz/2013/feb/linear-led-pwm\n\n')

    f.write('#include <stddef.h>\n')
    f.write('const size_t %s_size = %d;\n' % (TABLE_NAME, INPUT_SIZE+1))
    f.write('%s %s[] %s = {\n' % (INT_TYPE, TABLE_NAME, PROGMEM))
    f.write('\t')
    for i,L in enumerate(y):
        f.write('%f, ' % L)
        if i % 10 == 9:
            f.write('\n\t')
    f.write('\n};\n\n')