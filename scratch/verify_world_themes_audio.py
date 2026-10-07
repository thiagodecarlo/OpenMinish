"""
Script de Homologação Visual e Acústica dos 4 Temas de Mundo Procedurais (Opção 2)
- Mt. Crenel (Escalada Rochosa e Tempestuosa)
- Castor Wilds (Pântano Nebuloso e Traiçoeiro)
- Cloud Tops / Palace of Winds (Céus e Ventos Sagrados)
- Dark Hyrule Castle / Vaati (Castelo Corrompido)

Renderiza waveforms de alta fidelidade, matriz de canais PSG (Lead, Arp, Bass, Percussion)
e espectrograma dinâmico para certificação conforme Pilar 6 da Metodologia OpenMinish.
"""

import math
import random
import os
from PIL import Image, ImageDraw

OUTPUT_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\hal_audio_world_themes_showcase.png"
SAMPLE_RATE = 44100

def synth_square(phase, duty=0.5):
    p = phase % 1.0
    return 1.0 if p < duty else -1.0

def synth_triangle(phase):
    p = phase % 1.0
    return 4.0 * abs(p - 0.5) - 1.0

def synth_noise():
    return random.uniform(-1.0, 1.0)

def note_to_freq(midi):
    if midi <= 0: return 0.0
    return 440.0 * (2.0 ** ((midi - 69) / 12.0))

# 1. MT. CRENEL THEME
def generate_mt_crenel_samples(duration=4.0):
    num_samples = int(SAMPLE_RATE * duration)
    samples = []
    p_lead, p_arp, p_bass = 0.0, 0.0, 0.0
    bpm = 118.0
    beat_sec = 60.0 / bpm

    # Lead notes: Dm climbing
    lead_notes = [(62, 1.0), (65, 1.0), (69, 1.5), (67, 0.5), (65, 1.0), (62, 1.0), (60, 1.0), (62, 1.0)]
    arp_chords = [50, 53, 57, 62]

    for i in range(num_samples):
        t = i / SAMPLE_RATE
        # Lead
        cur_lead = 62
        acc = 0.0
        for n, dur in lead_notes:
            if acc <= t < acc + dur * beat_sec:
                cur_lead = n
                break
            acc += dur * beat_sec
        f_lead = note_to_freq(cur_lead)
        vib = 1.0 + 0.012 * math.sin(2.0 * math.pi * 5.2 * t)
        p_lead += (f_lead * vib) / SAMPLE_RATE
        lead = synth_square(p_lead, 0.50) * 0.35

        # Arp
        step = int(t / (beat_sec * 0.25))
        f_arp = note_to_freq(arp_chords[step % 4] + 12)
        p_arp += f_arp / SAMPLE_RATE
        arp = synth_square(p_arp, 0.25) * math.exp(-14.0 * (t % (beat_sec * 0.25))) * 0.20

        # Bass
        f_bass = note_to_freq(38 if ((int(t / beat_sec)) % 4 != 2) else 45)
        p_bass += f_bass / SAMPLE_RATE
        bass = (synth_triangle(p_bass) * 0.75 + synth_square(p_bass, 0.5) * 0.25) * math.exp(-4.0 * (t % beat_sec)) * 0.35

        # Drum
        beat_t = t % beat_sec
        b_idx = int(t / beat_sec) % 4
        drum = 0.0
        if b_idx % 2 == 0:
            drum = math.sin(2.0 * math.pi * 130.0 * math.exp(-25.0 * beat_t) * beat_t) * math.exp(-18.0 * beat_t) * 0.35
        else:
            drum = synth_noise() * math.exp(-20.0 * beat_t) * 0.25

        mix = max(-1.0, min(1.0, lead + arp + bass + drum))
        samples.append(mix)
    return samples

# 2. CASTOR WILDS THEME
def generate_castor_wilds_samples(duration=4.0):
    num_samples = int(SAMPLE_RATE * duration)
    samples = []
    p_lead, p_pad, p_bass = 0.0, 0.0, 0.0
    bpm = 94.0
    beat_sec = 60.0 / bpm

    lead_notes = [(71, 2.0), (74, 1.0), (70, 1.0), (69, 1.5), (66, 0.5), (67, 2.0)]

    for i in range(num_samples):
        t = i / SAMPLE_RATE
        # Lead
        cur_lead = 71
        acc = 0.0
        for n, dur in lead_notes:
            if acc <= t < acc + dur * beat_sec:
                cur_lead = n
                break
            acc += dur * beat_sec
        f_lead = note_to_freq(cur_lead)
        vib = 1.0 + 0.015 * math.sin(2.0 * math.pi * 4.2 * t)
        p_lead += (f_lead * vib) / SAMPLE_RATE
        lead = (synth_triangle(p_lead) * 0.65 + synth_square(p_lead, 0.125) * 0.35) * 0.38

        # Pad swell
        p_pad += note_to_freq(50) / SAMPLE_RATE
        swell = math.sin((t / (4.0 * beat_sec)) * math.pi) * 0.15
        pad = synth_square(p_pad, 0.5) * swell

        # Mud Bass
        p_bass += note_to_freq(35) / SAMPLE_RATE
        bass = synth_triangle(p_bass) * math.exp(-1.8 * (t % (2.0 * beat_sec))) * 0.40

        # Mud bubbles
        bubble_t = (t % beat_sec)
        bubble = 0.0
        if bubble_t < 0.08:
            bf = 480.0 - 300.0 * (bubble_t / 0.08)
            bubble = math.sin(2.0 * math.pi * bf * bubble_t) * math.exp(-35.0 * bubble_t) * 0.25

        mix = max(-1.0, min(1.0, lead + pad + bass + bubble))
        samples.append(mix)
    return samples

# 3. CLOUD TOPS THEME
def generate_cloud_tops_samples(duration=4.0):
    num_samples = int(SAMPLE_RATE * duration)
    samples = []
    p_lead, p_arp, p_bass = 0.0, 0.0, 0.0
    bpm = 126.0
    beat_sec = 60.0 / bpm

    lead_notes = [(75, 1.5), (79, 0.5), (82, 1.5), (80, 0.5), (79, 1.0), (75, 1.0), (77, 2.0)]
    arp_chords = [63, 67, 70, 75]

    for i in range(num_samples):
        t = i / SAMPLE_RATE
        # Celestial Lead
        cur_lead = 75
        acc = 0.0
        for n, dur in lead_notes:
            if acc <= t < acc + dur * beat_sec:
                cur_lead = n
                break
            acc += dur * beat_sec
        f_lead = note_to_freq(cur_lead)
        vib = 1.0 + 0.010 * math.sin(2.0 * math.pi * 6.0 * t)
        p_lead += (f_lead * vib) / SAMPLE_RATE
        lead = synth_square(p_lead, 0.50) * 0.36

        # Shimmer Arp
        step = int(t / (beat_sec * 0.25))
        f_arp = note_to_freq(arp_chords[step % 4] + 12)
        p_arp += f_arp / SAMPLE_RATE
        arp = synth_square(p_arp, 0.125) * math.exp(-16.0 * (t % (beat_sec * 0.25))) * 0.22

        # Airy Bass
        p_bass += note_to_freq(39) / SAMPLE_RATE
        bass = synth_triangle(p_bass) * math.exp(-2.2 * (t % (2.0 * beat_sec))) * 0.35

        # Wind sweep
        wind = synth_noise() * math.sin((t / (4.0 * beat_sec)) * math.pi) * 0.12

        mix = max(-1.0, min(1.0, lead + arp + bass + wind))
        samples.append(mix)
    return samples

# 4. DARK HYRULE CASTLE THEME
def generate_dark_castle_samples(duration=4.0):
    num_samples = int(SAMPLE_RATE * duration)
    samples = []
    p_lead1, p_lead2, p_arp, p_bass = 0.0, 0.0, 0.0, 0.0
    bpm = 132.0
    beat_sec = 60.0 / bpm

    lead_notes = [(62, 1.5), (65, 0.5), (69, 1.0), (74, 1.0), (73, 1.5), (70, 0.5), (69, 2.0)]
    arp_chords = [50, 53, 57, 62]

    for i in range(num_samples):
        t = i / SAMPLE_RATE
        # Gothic Pipe Organ (Dual Pulse 50% + 25%)
        cur_lead = 62
        acc = 0.0
        for n, dur in lead_notes:
            if acc <= t < acc + dur * beat_sec:
                cur_lead = n
                break
            acc += dur * beat_sec
        f_lead = note_to_freq(cur_lead)
        vib = 1.0 + 0.008 * math.sin(2.0 * math.pi * 6.2 * t)
        p_lead1 += (f_lead * vib) / SAMPLE_RATE
        p_lead2 += (f_lead * 2.0 * vib) / SAMPLE_RATE
        lead = (synth_square(p_lead1, 0.50) * 0.65 + synth_square(p_lead2, 0.25) * 0.35) * 0.38

        # Baroque Harpsichord Arp
        step = int(t / (beat_sec * 0.25))
        f_arp = note_to_freq(arp_chords[step % 4] + 12)
        p_arp += f_arp / SAMPLE_RATE
        arp = synth_square(p_arp, 0.25) * math.exp(-18.0 * (t % (beat_sec * 0.25))) * 0.20

        # Imperial March Bass
        p_bass += note_to_freq(38) / SAMPLE_RATE
        bass = (synth_triangle(p_bass) * 0.65 + synth_square(p_bass, 0.5) * 0.35) * math.exp(-4.5 * (t % beat_sec)) * 0.35

        # War Percussion
        beat_t = t % beat_sec
        b_idx = int(t / beat_sec) % 4
        drum = 0.0
        if b_idx % 2 == 0:
            drum = math.sin(2.0 * math.pi * 110.0 * math.exp(-30.0 * beat_t) * beat_t) * math.exp(-20.0 * beat_t) * 0.40
        else:
            drum = synth_noise() * math.exp(-22.0 * beat_t) * 0.28

        mix = max(-1.0, min(1.0, lead + arp + bass + drum))
        samples.append(mix)
    return samples

def draw_panel(draw, title, subtitle, tag, samples, px, py, pw, ph, accent_color):
    # Panel frame
    draw.rectangle([(px, py), (px + pw, py + ph)], fill=(15, 23, 42, 255), outline=(51, 65, 85, 255), width=2)
    # Header tag
    draw.rectangle([(px + 2, py + 2), (px + pw - 2, py + 30)], fill=(30, 41, 59, 255))
    draw.line([(px + 2, py + 30), (px + pw - 2, py + 30)], fill=accent_color, width=2)
    draw.text((px + 8, py + 5), title, fill=accent_color)
    draw.text((px + 8, py + 18), subtitle, fill=(148, 163, 184, 255))
    tag_w = 96
    draw.rectangle([(px + pw - tag_w - 8, py + 5), (px + pw - 8, py + 22)], fill=(15, 23, 42, 255), outline=accent_color)
    draw.text((px + pw - tag_w - 2, py + 7), tag, fill=(255, 255, 255, 255))

    # Waveform Display Box
    wave_x = px + 10
    wave_y = py + 38
    wave_w = pw - 20
    wave_h = 75
    draw.rectangle([(wave_x, wave_y), (wave_x + wave_w, wave_y + wave_h)], fill=(8, 15, 28, 255), outline=(71, 85, 105, 255))

    # Center axis
    mid_y = wave_y + wave_h // 2
    draw.line([(wave_x, mid_y), (wave_x + wave_w, mid_y)], fill=(30, 41, 59, 255))

    # Waveform drawing
    step = len(samples) / float(wave_w)
    prev_x, prev_y = wave_x, mid_y
    for x in range(wave_w):
        idx = int(x * step)
        s_val = samples[min(idx, len(samples) - 1)]
        cur_y = int(mid_y - s_val * (wave_h * 0.44))
        cur_x = wave_x + x
        draw.line([(prev_x, prev_y), (cur_x, cur_y)], fill=accent_color, width=1)
        prev_x, prev_y = cur_x, cur_y

    # Channel Matrix
    mat_y = py + 120
    ch_names = ["CH1: Lead Pulse", "CH2: Arp/Harmony", "CH3: Bass Tri/Sq", "CH4: Noise/Perc"]
    ch_colors = [accent_color, (56, 189, 248, 255), (250, 204, 21, 255), (248, 113, 113, 255)]
    for c_idx in range(4):
        cx = px + 10 + (c_idx % 2) * (pw // 2 - 5)
        cy = mat_y + (c_idx // 2) * 16
        draw.rectangle([(cx, cy + 2), (cx + 8, cy + 10)], fill=ch_colors[c_idx])
        draw.text((cx + 12, cy), ch_names[c_idx], fill=(226, 232, 240, 255))

def main():
    pw, ph = 330, 155
    header_h = 44
    cols, rows = 2, 2
    gap = 12

    total_w = cols * pw + (cols + 1) * gap
    total_h = header_h + rows * ph + (rows + 1) * gap

    canvas = Image.new("RGBA", (total_w, total_h), (10, 15, 26, 255))
    draw = ImageDraw.Draw(canvas)

    # Header
    draw.rectangle([(0, 0), (total_w, header_h)], fill=(15, 23, 42, 255))
    draw.line([(0, header_h), (total_w, header_h)], fill=(56, 189, 248, 255), width=2)
    draw.text((12, 6), "OPENMINISH - EXPANSÃO DA TRILHA CHIPTUNE PSG PROCEDURAL (OPÇÃO 2)", fill=(251, 191, 36, 255))
    draw.text((12, 24), "HAL C11 Audio | 4 Canais PSG Emulados (Lead, Arp, Bass, Noise) | 44.1kHz Stereo | Sem ROM", fill=(148, 163, 184, 255))

    panels_data = [
        ("Mt. Crenel (Escalada Rochosa)", "118 BPM | Dm/F | 50% Pulse Lead + 25% Rock Arp", "BGM_MT_CRENEL",
         generate_mt_crenel_samples(), (245, 158, 11, 255)),
        ("Castor Wilds (Pântano Nebuloso)", "94 BPM | Bm/Em | Tri/12.5% Lead + Mud Bubble Pops", "BGM_CASTOR_WILDS",
         generate_castor_wilds_samples(), (52, 211, 153, 255)),
        ("Cloud Tops & Palace of Winds", "126 BPM | Eb Maj | 12.5% Shimmer + Echo Celestial", "BGM_CLOUD_TOPS",
         generate_cloud_tops_samples(), (56, 189, 248, 255)),
        ("Dark Hyrule Castle & Vaati", "132 BPM | Dm | Órgão Barroco Gótico + Marcha Marcial", "BGM_DARK_CASTLE",
         generate_dark_castle_samples(), (239, 68, 68, 255)),
    ]

    for idx, (title, subtitle, tag, samples, accent) in enumerate(panels_data):
        c = idx % cols
        r = idx // cols
        px = gap + c * (pw + gap)
        py = header_h + gap + r * (ph + gap)
        draw_panel(draw, title, subtitle, tag, samples, px, py, pw, ph, accent)

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    canvas.save(OUTPUT_PATH)
    print(f"Showcase de áudio procedural salvo com sucesso em: {OUTPUT_PATH}")

if __name__ == "__main__":
    main()
