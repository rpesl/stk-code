#!/usr/bin/env python3
"""
Usage: ./update_plural_forms.py /path/to/po
"""

import sys
import os

if len(sys.argv) != 2:
    print('Usage: ./update_plural_forms.py /path/to/po')
    exit(-1)

pl_include = {}
for po in os.listdir(sys.argv[1]):
    if not po.endswith('.po'):
        continue
    po_file = open(sys.argv[1] + '/' + po, 'r')
    for line in po_file:
        if 'Plural-Forms:' in line:
            no_space_line = line.strip().replace(
                ' ', '').replace('"', '').replace('\\n', '')
            if no_space_line in pl_include:
                break
            pl = no_space_line.split(';')
            if len(pl) != 3:
                break
            pl_include[no_space_line] = ['tinygettext::PluralForms(' + pl[0].replace(
                'Plural-Forms:nplurals=', '') + \
                ', [](int n)-> unsigned int { return ' + pl[1].replace(
                'plural=', '') + '; })', po.replace('.po', '')]
            break

f = open('src/plural_forms_generated.hpp', 'w')
f.write('// Generated by update_plural_forms.py, do not edit\n')
f.write('#include <unordered_map>\n')
f.write('std::unordered_map<std::string, tinygettext::PluralForms> g_plural_forms = {\n')
for key, value in pl_include.items():
    f.write('{')
    f.write(' "' + key + '", ' + value[0])
    f.write(' },' + ' // ' + value[1] + '\n')
f.write('};\n')
