#!/usr/bin/env python3
# Genera src/font6x12.h a partir de spleen-6x12.bdf (Spleen, licencia BSD de 2 cláusulas,
# © Frederic Cambus; ver tools/LICENSE-spleen.txt).
#
# Solo incluye los caracteres que usa el asistente: ASCII imprimible y las letras con acento
# del español (á é í ó ú ü ñ, Á É Í Ó Ú Ü Ñ), ¿ ¡, y ° º ª. Cada glifo son 12 filas de 6 bits;
# el bit 7 de cada fila es la columna 0 (izquierda), como en el BDF.
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
BDF = os.path.join(HERE, 'spleen-6x12.bdf')
OUT = os.path.join(HERE, '..', 'src', 'font6x12.h')

EXTRA = 'áéíóúüñÁÉÍÓÚÜÑ¿¡°ºª'  # °, º y ª para temperaturas y ordinales
CODES = list(range(32, 127)) + [ord(c) for c in EXTRA]

txt = open(BDF, encoding='latin-1').read()
fbb = re.search(r'^FONTBOUNDINGBOX (\d+) (\d+) (-?\d+) (-?\d+)', txt, re.M)
FW, FH, FXOFF, FYOFF = map(int, fbb.groups())
assert (FW, FH) == (6, 12), 'se esperaba Spleen 6x12'
ASCENT = FH + FYOFF  # altura sobre la línea base (9)

glyphs = {}
for block in txt.split('STARTCHAR')[1:]:
    m = re.search(r'^ENCODING (\d+)', block, re.M)
    if not m:
        continue
    code = int(m.group(1))
    if code not in CODES:
        continue
    bbx = re.search(r'^BBX (\d+) (\d+) (-?\d+) (-?\d+)', block, re.M)
    w, h, xo, yo = map(int, bbx.groups())
    rows_hex = block.split('BITMAP', 1)[1].split('ENDCHAR', 1)[0].split()
    cell = [0] * FH  # filas de la celda 6x12, bit 7 = columna 0
    top = ASCENT - (yo + h)  # fila de la celda donde empieza el glifo
    for lr, hx in enumerate(rows_hex[:h]):
        v = int(hx, 16)  # ancho del BDF: 8 bits, usamos los 6 de la izquierda
        bits = 0
        for c in range(w):
            if (v >> (7 - c)) & 1:
                bits |= 1 << (7 - (xo + c))
        cell[top + lr] = bits & 0xFC
    glyphs[code] = cell

missing = [chr(c) for c in CODES if c not in glyphs]
assert not missing, f'faltan glifos: {missing}'

with open(OUT, 'w', encoding='utf-8') as f:
    f.write('// Fuente Spleen 6x12 para el asistente. Generada por tools/gen_font6x12.py.\n')
    f.write('// Spleen: licencia BSD de 2 cláusulas, © Frederic Cambus (tools/LICENSE-spleen.txt).\n')
    f.write('// Solo ASCII imprimible y caracteres del español. Cada fila es un byte (bit 7 = columna 0).\n\n')
    f.write('#pragma once\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('#define FONT6_W 6\n#define FONT6_H 12\n')
    f.write(f'#define FONT6_COUNT {len(CODES)}\n\n')
    f.write('static const uint32_t FONT6_CP[FONT6_COUNT] = {\n')
    for i in range(0, len(CODES), 8):
        f.write('    ' + ', '.join(f'0x{c:04X}' for c in CODES[i:i + 8]) + ',\n')
    f.write('};\n\n')
    f.write('static const uint8_t FONT6_ROWS[FONT6_COUNT][FONT6_H] = {\n')
    for c in CODES:
        rows = ', '.join(f'0x{b:02X}' for b in glyphs[c])
        # El comentario lleva solo el código: un carácter como \\ rompería la línea siguiente.
        f.write(f'    {{{rows}}},  // U+{c:04X}\n')
    f.write('};\n')

print('ok', OUT, len(CODES), 'glifos')
