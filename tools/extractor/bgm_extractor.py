#!/usr/bin/env python3
"""
tools/extractor/bgm_extractor.py - Extrator Autêntico de Trilhas Sonoras (BGM) do The Minish Cap
=============================================================================================
Extrai as músicas de fundo orquestradas originais diretamente da ROM de Game Boy Advance
utilizando o motor de renderização nativo MP2K (DirectSound + canais PSG) e exporta em WAV
estéreo de alta fidelidade para assets/audio/bgm/.
"""

import os
import sys
import argparse
import subprocess
import struct
import wave
import json
import urllib.request
import zipfile
import io

# Mapeamento oficial das faixas BGM do OpenMinish para os Song IDs do GBA
BGM_TRACKS = {
    "minish_woods": 29,          # BGM_MINISH_WOODS (Deepwood)
    "hyrule_overworld": 25,      # BGM_HYRULE_FIELD (Hyrule Overworld)
    "deepwood_shrine": 37,       # BGM_DEEPWOOD_SHRINE
    "boss_battle": 46,           # BGM_BOSS_THEME
    "hyrule_town": 32,           # BGM_HYRULE_TOWN
    "minish_village": 28,        # BGM_MINISH_VILLAGE (Picori Village)
    "title_theme": 3,            # BGM_TITLE_SCREEN
    "file_select": 7,            # BGM_FILE_SELECT (Fairy Fountain Harp)
    "cave_of_flames": 38,        # BGM_CAVE_OF_FLAMES (Monte Crenel)
    "fortress_of_winds": 39,     # BGM_FORTRESS_OF_WINDS
    "temple_of_droplets": 40,    # BGM_TEMPLE_OF_DROPLETS (Lago Hylia)
    "palace_of_winds": 41,       # BGM_PALACE_OF_WINDS (Nuvens)
    "dark_hyrule_castle": 35,    # BGM_DARK_HYRULE_CASTLE
    "royal_valley": 33,          # BGM_ROYAL_VALLEY (Cemitério)
    "elemental_sanctuary": 44,   # BGM_ELEMENTAL_SANCTUARY (Four Sword)
    "mt_crenel": 55,             # BGM_MT_CRENEL (Escalada)
    "crenel_storm": 30,          # BGM_CRENEL_STORM (Paredão)
    "castor_wilds": 31,          # BGM_CASTOR_WILDS (Pântano)
    "wind_ruins": 59,            # BGM_WIND_RUINS
    "cloud_tops": 34,            # BGM_CLOUD_TOPS (Topo das Nuvens)
    "swiftblade_dojo": 53,       # BGM_SWIFTBLADE_DOJO
    "cucco_minigame": 21,        # BGM_CUCCO_MINIGAME
    "house": 20,                 # BGM_HOUSE (Interiores)
    "picori_festival": 56,       # BGM_PICORI_FESTIVAL (Abertura)
}

AGBPLAY_DOWNLOAD_URL = "https://github.com/ipatix/agbplay/releases/download/nightly/v20260924-d209cce0449edfa61bbf15a9d22735ddfe09dd4f/nightly-agbplay-windows-mingw64-20260924.zip"

def ensure_agbplay_cli(bin_dir):
    """Garante que o executável agbplay-cli esteja disponível."""
    cli_path = os.path.join(bin_dir, "agbplay", "agbplay-cli.exe")
    if os.path.exists(cli_path):
        return cli_path
    
    cli_direct = os.path.join(bin_dir, "agbplay-cli.exe")
    if os.path.exists(cli_direct):
        return cli_direct

    print(f"[BGM EXTRACTOR] Baixando motor de renderizacao MP2K de {AGBPLAY_DOWNLOAD_URL}...")
    os.makedirs(bin_dir, exist_ok=True)
    req = urllib.request.Request(AGBPLAY_DOWNLOAD_URL, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as resp:
        zip_bytes = resp.read()
    
    with zipfile.ZipFile(io.BytesIO(zip_bytes)) as zf:
        zf.extractall(bin_dir)
    
    if os.path.exists(cli_path):
        return cli_path
    if os.path.exists(cli_direct):
        return cli_direct
    raise RuntimeError("Falha ao localizar agbplay-cli apos download!")

def find_default_rom():
    candidates = [
        "ROMS/Legend of Zelda, The - The Minish Cap (USA).gba",
        "ROMS/Legend of Zelda, The - The Minish Cap (Europe) (En,Fr,De,Es,It).gba",
        "ROMS/Zelda no Densetsu - Fushigi no Boushi (Japan).gba",
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    return None

import time

def convert_float32_to_pcm16_wav(in_wav, out_wav, max_duration_sec=60.0, fade_out_sec=1.5):
    """Converte arquivo WAV float32 IEEE (completo ou truncado) para WAV PCM 16-bit padrão."""
    with open(in_wav, 'rb') as f:
        _ = f.read(12)
        nChannels = 2
        nSamplesPerSec = 48000
        raw_data = None
        while True:
            chunk_hdr = f.read(8)
            if len(chunk_hdr) < 8:
                break
            chunk_id, chunk_size = struct.unpack('<4sI', chunk_hdr)
            if chunk_id == b'fmt ':
                fmt_data = f.read(chunk_size)
                _, nChannels, nSamplesPerSec, _, _, _ = struct.unpack('<HHIIHH', fmt_data[:16])
            elif chunk_id == b'data':
                if chunk_size > 0:
                    raw_data = f.read(chunk_size)
                else:
                    raw_data = f.read()
                break
            else:
                f.seek(chunk_size, 1)

    if not raw_data:
        raise ValueError(f"Chunk de dados de audio nao encontrado em {in_wav}")

    bytes_per_frame = nChannels * 4
    num_frames = len(raw_data) // bytes_per_frame
    if num_frames == 0:
        raise ValueError(f"Nenhum frame valido encontrado em {in_wav}")

    # Limita duracao maxima para faixas que continuam em loop
    max_frames = int(max_duration_sec * nSamplesPerSec)
    if num_frames > max_frames:
        num_frames = max_frames

    total_samples = num_frames * nChannels
    raw_floats = raw_data[:num_frames * bytes_per_frame]
    floats = struct.unpack(f'<{total_samples}f', raw_floats)

    fade_frames = int(fade_out_sec * nSamplesPerSec)
    fade_start = max(0, num_frames - fade_frames)

    pcm16 = bytearray(total_samples * 2)
    for i in range(num_frames):
        gain = 1.0
        if i >= fade_start and fade_frames > 0:
            gain = 1.0 - ((i - fade_start) / float(fade_frames))
        for ch in range(nChannels):
            val = floats[i * nChannels + ch] * gain
            if val > 1.0: val = 1.0
            elif val < -1.0: val = -1.0
            s16_val = int(val * 32767.0)
            idx = (i * nChannels + ch) * 2
            struct.pack_into('<h', pcm16, idx, s16_val)

    os.makedirs(os.path.dirname(os.path.abspath(out_wav)), exist_ok=True)
    with wave.open(out_wav, 'wb') as wf:
        wf.setnchannels(nChannels)
        wf.setsampwidth(2) # 16-bit
        wf.setframerate(nSamplesPerSec)
        wf.writeframes(pcm16)

def main():
    parser = argparse.ArgumentParser(description="Extrator Autentico de BGM de The Legend of Zelda: The Minish Cap")
    parser.add_argument("--rom", help="Caminho para a ROM do GBA")
    parser.add_argument("--output", default="assets/audio/bgm", help="Diretorio de saida para os arquivos WAV de BGM")
    parser.add_argument("--track", help="Extrair apenas uma faixa especifica (ex: minish_woods)")
    parser.add_argument("--force", action="store_true", help="Forca reextracao mesmo se ja existir")
    args = parser.parse_args()

    rom_path = args.rom or find_default_rom()
    if not rom_path or not os.path.exists(rom_path):
        print("[ERRO] Nenhuma ROM encontrada em ROMS/ ou especificada via --rom!")
        sys.exit(1)

    bin_dir = os.path.join(os.path.dirname(__file__), "bin")
    cli_path = ensure_agbplay_cli(bin_dir)

    os.makedirs(args.output, exist_ok=True)
    temp_dir = os.path.join(args.output, "_temp")
    os.makedirs(temp_dir, exist_ok=True)

    tracks_to_extract = BGM_TRACKS
    if args.track:
        if args.track not in BGM_TRACKS:
            print(f"[ERRO] Faixa desconhecida: {args.track}. Disponiveis: {list(BGM_TRACKS.keys())}")
            sys.exit(1)
        tracks_to_extract = {args.track: BGM_TRACKS[args.track]}

    print("====================================================================")
    print("   OpenMinish - Pipeline de Extracao Autentica de Trilhas BGM GBA   ")
    print("====================================================================")
    print(f"ROM Alvo: {rom_path}")
    print(f"Destino:  {args.output}")
    print(f"Total de Faixas: {len(tracks_to_extract)}")
    print("--------------------------------------------------------------------")

    manifest = {}
    success_count = 0

    for name, song_id in tracks_to_extract.items():
        raw_wav = os.path.join(temp_dir, f"{name}_raw.wav")
        out_wav = os.path.join(args.output, f"{name}.wav")

        if not args.force and os.path.exists(out_wav) and os.path.getsize(out_wav) > 100000:
            try:
                with wave.open(out_wav, 'rb') as wf:
                    duration_s = wf.getnframes() / float(wf.getframerate())
                    rate = wf.getframerate()
                    channels = wf.getnchannels()
                file_size = os.path.getsize(out_wav)
                print(f"[*] [{song_id:02d}] {name}... JA EXISTE ({duration_s:.1f}s, {file_size/1024/1024:.2f} MB, {rate}Hz)")
                manifest[name] = {
                    "song_id": song_id,
                    "file": f"{name}.wav",
                    "duration_seconds": round(duration_s, 2),
                    "sample_rate": rate,
                    "channels": channels,
                    "size_bytes": file_size
                }
                success_count += 1
                continue
            except Exception:
                pass

        print(f"[*] Extraindo [{song_id:02d}] {name}...", end=" ", flush=True)

        if os.path.exists(raw_wav):
            try: os.remove(raw_wav)
            except OSError: pass

        cmd = [cli_path, rom_path, "render", str(song_id), raw_wav, "master"]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        t0 = time.time()
        # Limite de 2.0s de processamento por faixa (equivalente a ~50s de audio em 48kHz)
        while time.time() - t0 < 2.0:
            if p.poll() is not None:
                break
            time.sleep(0.05)

        if p.poll() is None:
            p.kill()
            p.wait()

        if not os.path.exists(raw_wav) or os.path.getsize(raw_wav) < 1000:
            print(f"FALHA! (Arquivo nao gerado por agbplay)")
            continue

        try:
            convert_float32_to_pcm16_wav(raw_wav, out_wav, max_duration_sec=55.0, fade_out_sec=1.5)
            file_size = os.path.getsize(out_wav)
            
            with wave.open(out_wav, 'rb') as wf:
                duration_s = wf.getnframes() / float(wf.getframerate())
                rate = wf.getframerate()
                channels = wf.getnchannels()

            print(f"OK ({duration_s:.1f}s, {file_size/1024/1024:.2f} MB, {rate}Hz)")
            manifest[name] = {
                "song_id": song_id,
                "file": f"{name}.wav",
                "duration_seconds": round(duration_s, 2),
                "sample_rate": rate,
                "channels": channels,
                "size_bytes": file_size
            }
            success_count += 1
        except Exception as e:
            print(f"ERRO DE CONVERSAO: {e}")
        finally:
            if os.path.exists(raw_wav):
                try: os.remove(raw_wav)
                except OSError: pass

    # Remove pasta temporaria
    try:
        os.rmdir(temp_dir)
    except OSError:
        pass

    manifest_path = os.path.join(args.output, "manifest.json")
    with open(manifest_path, 'w', encoding='utf-8') as f:
        json.dump(manifest, f, indent=2)

    print("--------------------------------------------------------------------")
    print(f"[SUCESSO] {success_count}/{len(tracks_to_extract)} faixas BGM extraidas com exito!")
    print(f"Manifesto salvo em: {manifest_path}")

if __name__ == "__main__":
    main()
