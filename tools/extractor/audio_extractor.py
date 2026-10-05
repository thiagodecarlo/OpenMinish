#!/usr/bin/env python3
"""
tools/extractor/audio_extractor.py - Extrator Autêntico de Áudio e SFX de The Legend of Zelda: The Minish Cap
========================================================================================================
Extrai as amostras de áudio autênticas (DirectSound PCM 8-bit assinado -> WAV 16-bit PCM)
diretamente da ROM oficial (suporte completo a USA, Europa e Japão) para o pipeline
de áudio do OpenMinish (assets/audio/sfx/).

Uso:
    python tools/extractor/audio_extractor.py [--rom ROMS/Legend of Zelda, The - The Minish Cap (USA).gba]
"""

import os
import sys
import argparse
import struct
import wave
import json

# Offsets canônicos da Song Table por região
REGION_OFFSETS = {
    'USA': 0xA11DBC,
    'EUR': 0xB1D414,
    'JPN': 0x9F3D3C,
}

# Mapeamento dos nomes amigáveis dos principais SFX e vozes para OpenMinish
SFX_ALIAS_MAP = {
    # Vozes autênticas do Link (Fujiko Takimoto)
    117: 'sfx_ply_vo1',       # Grunt de ataque 1 (Yaaah!)
    118: 'sfx_ply_vo2',       # Grunt de ataque 2 (Haaah!)
    119: 'sfx_ply_vo3',       # Dano sofrido / Hurt (Ugh!)
    120: 'sfx_ply_vo4',       # Queda em abismo / Fall
    121: 'sfx_ply_vo5',       # Salto / Pulo (Hop!)
    122: 'sfx_ply_vo6',       # Ataque giratório carregado (Seiyaaa!)
    123: 'sfx_ply_vo7',       # Derrota / Game over (Uwaaah!)
    # Sons de movimento e interação do Link
    124: 'sfx_ply_jump',
    125: 'sfx_ply_land',
    127: 'sfx_ply_lift',
    130: 'sfx_water_walk',
    131: 'sfx_water_splash',
    132: 'sfx_fall_hole',
    134: 'sfx_ply_die',
    # Chimes e Fanfarras
    114: 'sfx_secret',
    115: 'sfx_secret_big',
    116: 'sfx_metal_clink',
    145: 'sfx_rupee_bounce',
    146: 'sfx_rupee_get',
    147: 'sfx_heart_bounce',
    148: 'sfx_heart_get',
    # Itens, Bombas, Armas
    250: 'sfx_sword_charge',
    251: 'sfx_sword_charge_finish',
    261: 'sfx_cucco_bell',
    264: 'sfx_button_depress',
    265: 'sfx_thud_heavy',
    288: 'sfx_bomb_explode',
    289: 'sfx_hit',
    300: 'sfx_item_get',
    318: 'sfx_chest_open',
    322: 'sfx_low_health',
    330: 'sfx_boss_hit',
    331: 'sfx_boss_die',
    332: 'sfx_boss_explode',
    352: 'sfx_lantern_on',
    353: 'sfx_lantern_off',
    354: 'sfx_sword_beam',
    356: 'sfx_heart_container_spawn',
    367: 'sfx_minish_shrink',
    368: 'sfx_minish_grow',
    371: 'sfx_ezlo_prompt',
    390: 'sfx_water_dive',
    398: 'sfx_switch_click',
    433: 'sfx_ice_slide',
    434: 'sfx_ice_stop',
    435: 'sfx_ice_melt',
    461: 'sfx_element_place',
    462: 'sfx_element_float',
    463: 'sfx_element_charge',
    465: 'sfx_element_infuse',
    470: 'sfx_cucco1',
    471: 'sfx_cucco2',
    472: 'sfx_cucco3',
    473: 'sfx_cucco4',
    474: 'sfx_cucco5',
    544: 'sfx_picolyte',
}

def detect_rom_region(rom_bytes):
    """Detecta a região da ROM através do header da ROM do GBA."""
    game_code = rom_bytes[0xAC:0xB0].decode('latin1', errors='ignore')
    if 'BZEE' in game_code or 'USA' in game_code:
        return 'USA'
    elif 'BZEP' in game_code:
        return 'EUR'
    elif 'BZEJ' in game_code:
        return 'JPN'
    
    # Fallback por verificação de integridade dos ponteiros da Song Table
    for reg, off in REGION_OFFSETS.items():
        if off + 16 < len(rom_bytes):
            ptr = int.from_bytes(rom_bytes[off:off+4], 'little')
            if 0x08000000 <= ptr < 0x09000000:
                h = ptr - 0x08000000
                if h + 8 < len(rom_bytes) and 1 <= rom_bytes[h] <= 16:
                    return reg
    return 'USA'

def extract_pcm_to_wav(pcm_raw, sample_rate, out_wav_path):
    """Converte PCM 8-bit assinado do GBA (-128..127) para WAV 16-bit PCM padrão."""
    pcm_16 = bytearray()
    for b in pcm_raw:
        s8 = b if b < 128 else b - 256
        s16 = s8 * 256
        pcm_16.extend(struct.pack('<h', s16))

    os.makedirs(os.path.dirname(os.path.abspath(out_wav_path)), exist_ok=True)
    with wave.open(out_wav_path, 'wb') as wf:
        wf.setnchannels(1)     # Mono
        wf.setsampwidth(2)     # 16-bit
        wf.setframerate(sample_rate)
        wf.writeframes(pcm_16)

def load_canonical_names():
    """Carrega nomes de enum de SFX do arquivo tmc_sound.h se disponível."""
    sound_h_path = os.path.join(os.path.dirname(__file__), '..', 'tmc_sound.h')
    names = {}
    if os.path.exists(sound_h_path):
        with open(sound_h_path, 'r', encoding='utf-8', errors='ignore') as f:
            text = f.read()
        start = text.find('typedef enum {')
        end = text.find('} Sound;')
        if start != -1 and end != -1:
            enum_str = text[start:end]
            lines = [l.strip() for l in enum_str.splitlines() if l.strip() and not l.strip().startswith('//') and not l.strip().startswith('typedef') and not l.strip().startswith('{')]
            val = 0
            for line in lines:
                parts = line.split(',')
                for part in parts:
                    part = part.strip()
                    if not part or part.startswith('//'): continue
                    if '=' in part:
                        k, v = part.split('=')
                        val = int(v.strip(), 0)
                        names[val] = k.strip()
                        val += 1
                    else:
                        names[val] = part
                        val += 1
    return names

def main():
    parser = argparse.ArgumentParser(description="Extrator de Áudio Autêntico do The Legend of Zelda: The Minish Cap")
    parser.add_argument("--rom", help="Caminho para o arquivo .gba da ROM oficial")
    parser.add_argument("--output", default="assets/audio/sfx", help="Diretório de saída para os arquivos WAV de SFX")
    parser.add_argument("--all", action="store_true", help="Extrair todas as amostras encontradas na ROM")
    args = parser.parse_args()

    # Procura ROM se não fornecida
    rom_path = args.rom
    if not rom_path:
        candidates = [
            "ROMS/Legend of Zelda, The - The Minish Cap (USA).gba",
            "ROMS/Legend of Zelda, The - The Minish Cap (Europe) (En,Fr,De,Es,It).gba",
            "ROMS/Zelda no Densetsu - Fushigi no Boushi (Japan).gba",
        ]
        for c in candidates:
            if os.path.exists(c):
                rom_path = c
                break

    if not rom_path or not os.path.exists(rom_path):
        print("[ERRO] Nenhuma ROM válida do GBA encontrada em ROMS/ ou informada via --rom!")
        sys.exit(1)

    print(f"[AUDIO EXTRACTOR] Lendo ROM: {rom_path}")
    with open(rom_path, 'rb') as f:
        rom = f.read()

    region = detect_rom_region(rom)
    song_table_offset = REGION_OFFSETS[region]
    print(f"[AUDIO EXTRACTOR] Região identificada: {region} | Song Table: 0x{song_table_offset:06X}")

    canonical_names = load_canonical_names()
    manifest = {}
    extracted_count = 0

    os.makedirs(args.output, exist_ok=True)

    # Itera por todas as entradas de SFX na Song Table (100 a 545)
    for song_idx in range(100, 546):
        entry_offset = song_table_offset + song_idx * 8
        header_ptr = int.from_bytes(rom[entry_offset:entry_offset+4], 'little')
        if not (0x08000000 <= header_ptr < 0x09000000):
            continue
        hdr_offset = header_ptr - 0x08000000
        vg_ptr = int.from_bytes(rom[hdr_offset+4:hdr_offset+8], 'little')
        if not (0x08000000 <= vg_ptr < 0x09000000):
            continue
        vg_offset = vg_ptr - 0x08000000

        # Encontra amostra DirectSound no VoiceGroup
        for inst_idx in range(16):
            if vg_offset + inst_idx * 12 + 12 > len(rom):
                break
            smp_ptr = int.from_bytes(rom[vg_offset + inst_idx * 12 + 4: vg_offset + inst_idx * 12 + 8], 'little')
            if not (0x08000000 <= smp_ptr < 0x09000000):
                continue
            smp_offset = smp_ptr - 0x08000000
            if smp_offset + 16 > len(rom):
                continue

            length = int.from_bytes(rom[smp_offset+12:smp_offset+16], 'little')
            freq = int.from_bytes(rom[smp_offset+4:smp_offset+8], 'little') // 1024
            loop_start = int.from_bytes(rom[smp_offset+8:smp_offset+12], 'little')

            if 16 <= length <= 600000 and 4000 <= freq <= 48000:
                raw_pcm = rom[smp_offset+16 : smp_offset+16+length]
                if len(raw_pcm) < length:
                    continue

                # Determina o nome do arquivo
                canon_name = canonical_names.get(song_idx, f"sfx_{song_idx}").lower()
                alias = SFX_ALIAS_MAP.get(song_idx)
                
                # Nome base
                target_filename = alias if alias else canon_name
                out_path = os.path.join(args.output, f"{target_filename}.wav")

                extract_pcm_to_wav(raw_pcm, freq, out_path)
                extracted_count += 1

                manifest[target_filename] = {
                    "song_idx": song_idx,
                    "sample_ptr": f"0x{smp_ptr:08X}",
                    "freq": freq,
                    "length": length,
                    "loop_start": loop_start,
                    "filename": f"{target_filename}.wav"
                }

                # Se houver alias e for diferente do nome canônico, também cria o symlink/cópia
                if alias and alias != canon_name:
                    canon_path = os.path.join(args.output, f"{canon_name}.wav")
                    extract_pcm_to_wav(raw_pcm, freq, canon_path)

                break  # Usa a amostra principal deste som

    manifest_path = os.path.join(os.path.dirname(args.output), "sfx_manifest.json")
    with open(manifest_path, 'w', encoding='utf-8') as f:
        json.dump(manifest, f, indent=2)

    print(f"[AUDIO EXTRACTOR] Sucesso! {extracted_count} efeitos sonoros autênticos extraídos para '{args.output}'.")
    print(f"[AUDIO EXTRACTOR] Manifesto gerado em '{manifest_path}'.")

if __name__ == "__main__":
    main()
