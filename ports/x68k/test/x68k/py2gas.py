#!/usr/bin/env python3
import sys
import re

def convert(line):
    if re.match(r'\s*#', line):
        return f'; {line}'
    if re.match(r'\w', line):
        return None
    m = re.match(r'\s+(\w+)\((.*)\)', line)
    if not m:
        return ''

    line = m[2]
    opc = m[1]
    opr = ''

    if opc == 'label':
        return f'{line}:'

    while line:
        line = line.lstrip()
        if line.startswith(','):
            opr += ','
            line = line[1:]
        elif m := re.match(r'([ad][0-7]|fp|sp|ccr|sr|usp)', line):
            opr += f'%{m[1]}'
            line = line[len(m[0]):]
        elif m := re.match(r'([-+]?\d\w*)',line):
            opr += f'#{m[1]}'
            line = line[len(m[0]):]
        elif m := re.match(r'(\w+)',line):
            opr += f'{m[1]}'
            line = line[len(m[0]):]
        elif m := re.match(r'\[(a[0-7]|fp|sp)(\.(inc|dec))?\]', line):
            opr += f'%{m[1]}@'
            if m[2] == '.inc':
                opr += '+'
            elif m[2] == '.dec':
                opr += '-'
            line = line[len(m[0]):]
        elif line[0] == '[':
            line = line[1:]
            dbase = ''
            ddisp = ''
            dreg = ''
            while line:
                if line[0] == ']':
                    line = line[1:]
                    if m := re.match(r'\.([wl])', line):
                        ddisp += f':{m[1]}'
                        line = line[len(m[0]):]
                    break
                elif line[0] == ',':
                    line = line[1:]
                elif m := re.match(r'(a[0-7]|fp|sp|pc)[^.]', line):
                    dbase = f'%{m[1]}'
                    line = line[len(m[1]):]
                elif m := re.match(r'([ad][0-7]|fp|sp)\.([wl])', line):
                    dreg = f'%{m[1]}:{m[2]}'
                    line = line[len(m[0]):]
                elif m := re.match(r'([-+]?\w+)\.([wl])', line):
                    ddisp = f'{m[1]}:{m[2]}'
                    line = line[len(m[0]):]
                elif m := re.match(r'([-+]?\w+)', line):
                    ddisp = m[1]
                    line = line[len(m[0]):]
                else:
                    ddisp = line + '# error'
                    line = ''
            if dbase:
                opr += f'{dbase}@('
                if ddisp:
                    opr += ddisp
                    if dreg:
                        opr += ','
                if dreg:
                    opr += dreg
                opr += ')'
            else:
                opr += ddisp
        elif line[0] == '{':
            line = line[1:]
            while line:
                if line[0] == '}':
                    line = line[1:]
                    break
                elif line[0] in ',-/':
                    opr += line[0]
                    line = line[1:]
                elif m := re.match(r'([ad][0-7]|fp|sp)', line):
                    opr += f'%{m[1]}'
                    line = line[len(m[0]):]
                else:
                    opr += line + '# error'
                    line = ''
        else:
            opr += line + '\t# error'
            line = ''

    return f'\t{opc}\t{opr}'

def readfile(f):
    c=0
    l=0
    for line in f:
        l += 1
        if c == 0:
            if line.startswith('@micropython.asm_m68k'):
                c = 1
        elif c == 1:
            if line.startswith('def '):
                c = 2
            else:
                c = 0
        else:
            s = convert(line.rstrip('\r\n'))
            if s != None:
                print(s)
            else:
                c = 0

def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1], "r") as f:
            readfile(f)
    else:
        readfile(sys.stdin)

if __name__ == "__main__":
    main()
