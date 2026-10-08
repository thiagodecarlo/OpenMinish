#!/usr/bin/env python3
"""
TMC Strings Packer - Converts TMC JSON translations to binary string tables.
Faithfully implements the binary format used by The Minish Cap (GBA).
"""

import sys
import os
import json
import struct

# GBA TMC Charset Mapping (standard ASCII + European/Portuguese accents)
CHAR_MAP = {
    '\n': 0x0A, '\r': 0x0D, ' ': 0x20, '!': 0x21, '"': 0x22, '#': 0x23, '$': 0x24,
    '%': 0x25, '&': 0x26, "'": 0x27, '(': 0x28, ')': 0x29, '*': 0x2A, '+': 0x2B,
    ',': 0x2C, '-': 0x2D, '.': 0x2E, '/': 0x2F, '0': 0x30, '1': 0x31, '2': 0x32,
    '3': 0x33, '4': 0x34, '5': 0x35, '6': 0x36, '7': 0x37, '8': 0x38, '9': 0x39,
    ':': 0x3A, ';': 0x3B, '<': 0x3C, '=': 0x3D, '>': 0x3E, '?': 0x3F, '@': 0x40,
    'A': 0x41, 'B': 0x42, 'C': 0x43, 'D': 0x44, 'E': 0x45, 'F': 0x46, 'G': 0x47,
    'H': 0x48, 'I': 0x49, 'J': 0x4A, 'K': 0x4B, 'L': 0x4C, 'M': 0x4D, 'N': 0x4E,
    'O': 0x4F, 'P': 0x50, 'Q': 0x51, 'R': 0x52, 'S': 0x53, 'T': 0x54, 'U': 0x55,
    'V': 0x56, 'W': 0x57, 'X': 0x58, 'Y': 0x59, 'Z': 0x5A, '[': 0x5B, ']': 0x5D,
    '^': 0x5E, '_': 0x5F, '`': 0x60, 'a': 0x61, 'b': 0x62, 'c': 0x63, 'd': 0x64,
    'e': 0x65, 'f': 0x66, 'g': 0x67, 'h': 0x68, 'i': 0x69, 'j': 0x6A, 'k': 0x6B,
    'l': 0x6C, 'm': 0x6D, 'n': 0x6E, 'o': 0x6F, 'p': 0x70, 'q': 0x71, 'r': 0x72,
    's': 0x73, 't': 0x74, 'u': 0x75, 'v': 0x76, 'w': 0x77, 'x': 0x78, 'y': 0x79,
    'z': 0x7A, '⋯': 0x85, '“': 0x93, '”': 0x94, '’': 0x92, '‘': 0x91, '♪': 0xA3,
    'ª': 0xAA, 'º': 0xBA, '¡': 0xA1, '¿': 0xBF, '«': 0xAB, '»': 0xBB,
    # Portuguese / Latin uppercase
    'À': 0xC0, 'Á': 0xC1, 'Â': 0xC2, 'Ã': 0xC3, 'Ä': 0xC4, 'Ç': 0xC7,
    'È': 0xC8, 'É': 0xC9, 'Ê': 0xCA, 'Ë': 0xCB, 'Ì': 0xCC, 'Í': 0xCD,
    'Ñ': 0xD1, 'Ò': 0xD2, 'Ó': 0xD3, 'Ô': 0xD4, 'Õ': 0xD5, 'Ö': 0xD6,
    'Ù': 0xD9, 'Ú': 0xDA, 'Û': 0xDB, 'Ü': 0xDC,
    # Portuguese / Latin lowercase
    'à': 0xE0, 'á': 0xE1, 'â': 0xE2, 'ã': 0xE3, 'ä': 0xE4, 'ç': 0xE7,
    'è': 0xE8, 'é': 0xE9, 'ê': 0xEA, 'ë': 0xEB, 'ì': 0xEC, 'í': 0xED,
    'ñ': 0xF1, 'ò': 0xF2, 'ó': 0xF3, 'ô': 0xF4, 'õ': 0xF5, 'ö': 0xF6,
    'ù': 0xF9, 'ú': 0xFA, 'û': 0xFB, 'ü': 0xFC,
}

COLORS = {'White': 0, 'Red': 1, 'Green': 2, 'Blue': 3, 'Yellow': 4}
INPUTS = {'A': 0, 'B': 1, 'Left': 2, 'Right': 3, 'DUp': 4, 'DDown': 5, 'DLeft': 6, 'DRight': 7, 'Dpad': 8, 'Select': 9, 'Start': 10}

def encode_tmc_string(s):
    out = bytearray()
    i = 0
    while i < len(s):
        if s[i] == '{':
            end = s.find('}', i)
            if end == -1:
                raise ValueError(f"Unclosed tag in: {s[i:]}")
            tag = s[i+1:end]
            i = end + 1
            if tag.startswith('Color:'):
                cname = tag[6:]
                out.extend([0x02, COLORS[cname]])
            elif tag.startswith('Sound:'):
                parts = tag[6:].split(':')
                out.extend([0x03, int(parts[0], 16), int(parts[1], 16)])
            elif tag.startswith('Choice:'):
                parts = tag[7:].split(':')
                c0 = int(parts[0], 16)
                if c0 == 0xFF:
                    out.extend([0x05, 0xFF])
                else:
                    out.extend([0x05, c0, int(parts[1], 16)])
            elif tag == 'Player':
                out.extend([0x06, 0x00])
            elif tag.startswith('Var:'):
                v = int(tag[4:], 16)
                out.extend([0x06, v])
            elif tag.startswith('Key:'):
                k = tag[4:]
                out.extend([0x0C, INPUTS[k]])
            elif tag.startswith('Symbol:'):
                sym = int(tag[7:], 16)
                out.extend([0x0F, sym])
            elif ':' in tag:
                # Raw hex bytecode tag like {04:10:00} or {01:05}
                parts = tag.split(':')
                for p in parts:
                    out.append(int(p, 16))
            else:
                out.append(int(tag, 16))
        else:
            ch = s[i]
            if ch in CHAR_MAP:
                out.append(CHAR_MAP[ch])
            elif ch in ('‘', '’', '`', '´'):
                out.append(0x27)
            elif ch in ('“', '”'):
                out.append(0x22)
            elif ch == '—' or ch == '–':
                out.append(0x2D)
            else:
                out.append(ord(ch) & 0xFF)
            i += 1
    out.append(0)  # Null terminator
    return bytes(out)

def pack_string_table(json_path, bin_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        categories = json.load(f)

    category_count = len(categories)
    # Root table has category_count * 4 bytes
    root_table = bytearray(category_count * 4)
    table_data = bytearray()

    for cat_idx, strings in enumerate(categories):
        cat_offset = len(root_table) + len(table_data)
        struct.pack_into('<I', root_table, cat_idx * 4, cat_offset)

        string_count = len(strings)
        string_table = bytearray(string_count * 4)
        string_bytes = bytearray()

        for str_idx, s in enumerate(strings):
            str_offset = string_count * 4 + len(string_bytes)
            struct.pack_into('<I', string_table, str_idx * 4, str_offset)
            string_bytes.extend(encode_tmc_string(s))

        # Pad category to 16 bytes
        cat_blob = string_table + string_bytes
        while len(cat_blob) % 16 != 0:
            cat_blob.append(0xFF)

        table_data.extend(cat_blob)

    full_blob = root_table + table_data
    while len(full_blob) % 16 != 0:
        full_blob.append(0xFF)

    with open(bin_path, 'wb') as f:
        f.write(full_blob)

    print(f"Packed {json_path} -> {bin_path} ({len(full_blob)} bytes, {category_count} categories).")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python pack_tmc_strings.py <source.json> <dest.bin>")
        sys.exit(1)
    pack_string_table(sys.argv[1], sys.argv[2])
