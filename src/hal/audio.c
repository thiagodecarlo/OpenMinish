#include "hal/audio.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * ============================================================================
 * src/hal/audio.c - Implementação da Camada de Áudio, Mixer e Sintetizador Retro
 * ============================================================================
 */

#define AUDIO_SAMPLE_RATE    44100
#define AUDIO_CHANNELS       2       // Estéreo
#define AUDIO_BUFFER_SIZE    512     // ~11.6ms de latência (altamente responsivo)
#define MAX_CONCURRENT_VOICES 16

#define PI_F 3.14159265358979323846f

typedef struct {
    s16*   samples;
    u32    total_frames;
    float  current_frame;
    float  volume;
    float  pitch;
    bool   is_active;
    bool   is_looping;
    bool   is_stereo;
    bool   free_on_finish;
} Voice;

// Amostras pré-sintetizadas dos efeitos proceduais (alocadas na inicialização)
typedef struct {
    s16* samples;
    u32  total_frames;
    bool is_stereo;
} PrecalcSfx;

static SDL_AudioDeviceID s_audio_device = 0;
static SDL_AudioSpec     s_audio_spec;

static Voice      s_voices[MAX_CONCURRENT_VOICES];
static PrecalcSfx s_precalc_sfx[SOUND_COUNT];

static Voice s_bgm_voice = { 0 };

static float s_vol_master = 0.85f;
static float s_vol_sfx    = 0.90f;
static float s_vol_bgm    = 0.70f;
static bool  s_audio_ready = false;
static BgmTrack s_current_bgm_track = BGM_NONE;

// ----------------------------------------------------------------------------
// SINTETIZADOR PROCEDURAL RETRO (EMULAÇÃO DE PSG E DIRECTSOUND)
// ----------------------------------------------------------------------------

static float synth_square_wave(float phase, float duty) {
    float p = fmodf(phase, 1.0f);
    if (p < 0.0f) p += 1.0f;
    return (p < duty) ? 1.0f : -1.0f;
}

static float synth_triangle_wave(float phase) {
    float p = fmodf(phase, 1.0f);
    if (p < 0.0f) p += 1.0f;
    return 4.0f * fabsf(p - 0.5f) - 1.0f;
}

static float synth_noise(void) {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

static float note_to_freq(int midi_note) {
    if (midi_note <= 0) return 0.0f;
    return 440.0f * powf(2.0f, (float)(midi_note - 69) / 12.0f);
}

typedef struct {
    u8    note;
    float duration;
} NoteEvent;

static void synth_generate_all_sfx(void) {
    // 1. SOUND_SWORD_SLASH: Golpe de espada (Ruído branco com sweep de pitch e decaimento exponencial)
    {
        int num_frames = (int)(AUDIO_SAMPLE_RATE * 0.16f); // 160ms
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));
        float env = 1.0f;
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            env = expf(-22.0f * t); // Decaimento rápido
            float sample = synth_noise() * env * 0.9f;
            // Adiciona um harmônico cortante metálico sutil
            float tone = sinf(2.0f * PI_F * (1200.0f - 800.0f * (t / 0.16f)) * t) * env * 0.3f;
            float total = (sample + tone) * 32000.0f;
            if (total > 32767.0f) total = 32767.0f;
            if (total < -32768.0f) total = -32768.0f;
            buf[i] = (s16)total;
        }
        s_precalc_sfx[SOUND_SWORD_SLASH].samples = buf;
        s_precalc_sfx[SOUND_SWORD_SLASH].total_frames = num_frames;
        s_precalc_sfx[SOUND_SWORD_SLASH].is_stereo = false;
    }

    // 2. SOUND_SWORD_HIT: Impacto metálico sólido (Clink de dois tons harmônicos)
    {
        int num_frames = (int)(AUDIO_SAMPLE_RATE * 0.14f);
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float env = expf(-35.0f * t);
            float f1 = sinf(2.0f * PI_F * 1760.0f * t);
            float f2 = sinf(2.0f * PI_F * 2340.0f * t);
            float noise = synth_noise() * 0.3f;
            float total = (f1 * 0.5f + f2 * 0.4f + noise) * env * 30000.0f;
            if (total > 32767.0f) total = 32767.0f;
            if (total < -32768.0f) total = -32768.0f;
            buf[i] = (s16)total;
        }
        s_precalc_sfx[SOUND_SWORD_HIT].samples = buf;
        s_precalc_sfx[SOUND_SWORD_HIT].total_frames = num_frames;
        s_precalc_sfx[SOUND_SWORD_HIT].is_stereo = false;
    }

    // 3. SOUND_FOOTSTEP: Passo macio no chão (Thud amortecido de baixa frequência)
    {
        int num_frames = (int)(AUDIO_SAMPLE_RATE * 0.05f); // 50ms
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float env = expf(-65.0f * t);
            float low_freq = sinf(2.0f * PI_F * (110.0f - 40.0f * (t / 0.05f)) * t);
            float tap = synth_noise() * 0.25f;
            float total = (low_freq * 0.8f + tap) * env * 22000.0f;
            if (total > 32767.0f) total = 32767.0f;
            if (total < -32768.0f) total = -32768.0f;
            buf[i] = (s16)total;
        }
        s_precalc_sfx[SOUND_FOOTSTEP].samples = buf;
        s_precalc_sfx[SOUND_FOOTSTEP].total_frames = num_frames;
        s_precalc_sfx[SOUND_FOOTSTEP].is_stereo = false;
    }

    // 4. SOUND_ROLL: Rolar / Esquiva ágil (Whoosh aerodinâmico)
    {
        int num_frames = (int)(AUDIO_SAMPLE_RATE * 0.22f);
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float progress = t / 0.22f;
            // Envelope em formato de sino (suave no início e no final)
            float env = sinf(progress * PI_F);
            float f = 200.0f + 500.0f * sinf(progress * PI_F);
            float tone = sinf(2.0f * PI_F * f * t) * 0.4f;
            float noise = synth_noise() * 0.6f;
            float total = (tone + noise) * env * 26000.0f;
            if (total > 32767.0f) total = 32767.0f;
            if (total < -32768.0f) total = -32768.0f;
            buf[i] = (s16)total;
        }
        s_precalc_sfx[SOUND_ROLL].samples = buf;
        s_precalc_sfx[SOUND_ROLL].total_frames = num_frames;
        s_precalc_sfx[SOUND_ROLL].is_stereo = false;
    }

    // 5. SOUND_HEART_BEEP: Chime clássico de vida baixa
    {
        int num_frames = (int)(AUDIO_SAMPLE_RATE * 0.18f);
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float freq = (t < 0.09f) ? 880.0f : 1174.66f; // Lá5 e Ré6
            float env = expf(-18.0f * fmodf(t, 0.09f));
            float phase = freq * t;
            float wave = synth_square_wave(phase, 0.50f) * env * 22000.0f;
            buf[i] = (s16)wave;
        }
        s_precalc_sfx[SOUND_HEART_BEEP].samples = buf;
        s_precalc_sfx[SOUND_HEART_BEEP].total_frames = num_frames;
        s_precalc_sfx[SOUND_HEART_BEEP].is_stereo = false;
    }

    // 6. SOUND_SECRET: O lendário acorde de segredo revelado (8 notas de Zelda)
    // G5 -> F#5 -> D#5 -> A4 -> G#4 -> E5 -> G#5 -> C6
    {
        float notes[] = {
            783.99f, // Sol 5
            739.99f, // Fá# 5
            622.25f, // Ré# 5
            440.00f, // Lá 4
            415.30f, // Sol# 4
            659.25f, // Mi 5
            830.61f, // Sol# 5
            1046.50f // Dó 6 (sustentado)
        };
        float note_dur = 0.105f; // 105ms por nota
        float final_dur = 0.400f; // nota final sustentada
        float total_time = (7 * note_dur) + final_dur;
        int num_frames = (int)(AUDIO_SAMPLE_RATE * total_time);
        s16* buf = (s16*)malloc(num_frames * sizeof(s16));

        float phase = 0.0f;
        for (int i = 0; i < num_frames; i++) {
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            int note_idx = (int)(t / note_dur);
            if (note_idx > 7) note_idx = 7;

            float cur_freq = notes[note_idx];
            float note_time = (note_idx < 7) ? fmodf(t, note_dur) : (t - (7 * note_dur));
            float env = (note_idx < 7) ? expf(-8.0f * note_time) : expf(-4.0f * note_time);

            // Adiciona vibrato sutil na nota final para sonoridade idêntica ao Game Boy
            if (note_idx == 7) {
                cur_freq += sinf(2.0f * PI_F * 6.0f * note_time) * 12.0f;
            }

            phase += cur_freq / (float)AUDIO_SAMPLE_RATE;
            float wave = synth_square_wave(phase, 0.25f); // Onda quadrada clássica de 25% duty cycle do PSG
            float total = wave * env * 25000.0f;
            buf[i] = (s16)total;
        }
        s_precalc_sfx[SOUND_SECRET].samples = buf;
        s_precalc_sfx[SOUND_SECRET].total_frames = num_frames;
        s_precalc_sfx[SOUND_SECRET].is_stereo = false;
    }
}

// ----------------------------------------------------------------------------
// SINTETIZADORES DE TRILHAS SONORAS (BGM CHIPTUNE POLIFÔNICO)
// ----------------------------------------------------------------------------

static s16* synth_generate_minish_woods(u32* out_total_frames) {
    float bpm = 100.0f;
    float beat_sec = 60.0f / bpm;
    float total_sec = beat_sec * 3.0f * 16.0f; // 16 compassos de 3 batidas = 28.8s
    u32 total_frames = (u32)(AUDIO_SAMPLE_RATE * total_sec);

    float* mix_l = (float*)calloc(total_frames, sizeof(float));
    float* mix_r = (float*)calloc(total_frames, sizeof(float));
    if (!mix_l || !mix_r) {
        if (mix_l) free(mix_l);
        if (mix_r) free(mix_r);
        return NULL;
    }

    // Canal 1: Melodia Principal (Flauta / Ocarina Minish com vibrato expressivo)
    static const NoteEvent s_woods_lead[] = {
        // Compassos 1 e 2 (Dm)
        { 74, 1.0f }, { 77, 1.0f }, { 81, 1.0f }, { 86, 1.5f }, { 84, 0.5f }, { 81, 1.0f },
        // Compassos 3 e 4 (Bb)
        { 82, 2.0f }, { 81, 1.0f }, { 79, 3.0f },
        // Compassos 5 e 6 (C)
        { 76, 1.0f }, { 79, 1.0f }, { 82, 1.0f }, { 84, 1.5f }, { 82, 0.5f }, { 79, 1.0f },
        // Compassos 7 e 8 (Am)
        { 81, 2.0f }, { 77, 1.0f }, { 74, 3.0f },
        // Compassos 9 e 10 (Bb)
        { 77, 1.0f }, { 81, 1.0f }, { 86, 1.0f }, { 88, 2.0f }, { 86, 1.0f },
        // Compassos 11 e 12 (Gm)
        { 82, 1.5f }, { 84, 0.5f }, { 86, 1.0f }, { 81, 3.0f },
        // Compassos 13 e 14 (C)
        { 79, 1.0f }, { 82, 1.0f }, { 86, 1.0f }, { 84, 1.5f }, { 82, 0.5f }, { 81, 1.0f },
        // Compassos 15 e 16 (A7 -> Dm)
        { 79, 2.0f }, { 76, 1.0f }, { 74, 3.0f }
    };
    int lead_count = (int)(sizeof(s_woods_lead) / sizeof(s_woods_lead[0]));

    float cur_time = 0.0f;
    for (int n = 0; n < lead_count; n++) {
        float dur_sec = s_woods_lead[n].duration * beat_sec;
        u32 start_frame = (u32)(cur_time * AUDIO_SAMPLE_RATE);
        u32 num_frames = (u32)(dur_sec * AUDIO_SAMPLE_RATE);
        float base_freq = note_to_freq(s_woods_lead[n].note);
        float phase = 0.0f;

        for (u32 i = 0; i < num_frames; i++) {
            u32 idx = (start_frame + i) % total_frames;
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float t_note = t / dur_sec;

            float att = (t < 0.04f) ? (t / 0.04f) : 1.0f;
            float dec = 1.0f - (t_note * 0.25f);
            if (t_note > 0.82f) {
                dec *= (1.0f - t_note) / 0.18f;
                if (dec < 0.0f) dec = 0.0f;
            }

            float cur_freq = base_freq;
            if (t > 0.16f && base_freq > 0.0f) {
                cur_freq += sinf(2.0f * PI_F * 5.4f * (t - 0.16f)) * (base_freq * 0.016f);
            }

            phase += cur_freq / (float)AUDIO_SAMPLE_RATE;
            float flutewave = (synth_triangle_wave(phase) * 0.65f + synth_square_wave(phase, 0.25f) * 0.35f);
            float sample = flutewave * att * dec * 0.40f;

            mix_l[idx] += sample * 0.52f;
            mix_r[idx] += sample * 0.48f;
        }
        cur_time += dur_sec;
    }

    // Canal 2: Harpa das Fadas e Arpeggios Magicos (com eco estereo)
    static const u8 s_woods_harp[8][6] = {
        { 62, 65, 69, 74, 69, 65 }, // Dm (D4, F4, A4, D5, A4, F4)
        { 62, 65, 70, 74, 70, 65 }, // Bb (D4, F4, Bb4, D5, Bb4, F4)
        { 60, 64, 67, 72, 67, 64 }, // C  (C4, E4, G4, C5, G4, E4)
        { 57, 60, 64, 69, 64, 60 }, // Am (A3, C4, E4, A4, E4, C4)
        { 62, 65, 70, 74, 70, 65 }, // Bb (D4, F4, Bb4, D5, Bb4, F4)
        { 58, 62, 67, 70, 67, 62 }, // Gm (Bb3, D4, G4, Bb4, G4, D4)
        { 60, 64, 67, 72, 67, 64 }, // C  (C4, E4, G4, C5, G4, E4)
        { 57, 61, 64, 69, 64, 61 }  // A7 (A3, C#4, E4, A4, E4, C#4)
    };

    float note_step_sec = beat_sec * 0.5f;
    for (int bar = 0; bar < 16; bar++) {
        int pattern_idx = bar / 2;
        for (int step = 0; step < 6; step++) {
            u8 harp_note = s_woods_harp[pattern_idx][step];
            float harp_time = (bar * 3.0f * beat_sec) + (step * note_step_sec);
            u32 start_frame = (u32)(harp_time * AUDIO_SAMPLE_RATE);
            u32 num_frames = (u32)(0.40f * AUDIO_SAMPLE_RATE);
            float freq = note_to_freq(harp_note);
            float phase = 0.0f;

            float pan_l = (step % 2 == 0) ? 0.70f : 0.30f;
            float pan_r = 1.0f - pan_l;

            for (u32 i = 0; i < num_frames; i++) {
                u32 idx = (start_frame + i) % total_frames;
                float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                float env = expf(-11.0f * t);

                phase += freq / (float)AUDIO_SAMPLE_RATE;
                float chime = sinf(2.0f * PI_F * phase) * 0.65f + synth_square_wave(phase, 0.125f) * 0.35f;
                float sample = chime * env * 0.18f;

                mix_l[idx] += sample * pan_l;
                mix_r[idx] += sample * pan_r;
            }
        }
    }

    // Canal 3: Baixo Acustico de Floresta (Onda Triangular PSG suave)
    static const NoteEvent s_woods_bass[] = {
        { 50, 2.0f }, { 45, 1.0f }, { 50, 3.0f }, // Dm (D3, A2, D3)
        { 46, 2.0f }, { 41, 1.0f }, { 46, 3.0f }, // Bb (Bb2, F2, Bb2)
        { 48, 2.0f }, { 43, 1.0f }, { 48, 3.0f }, // C  (C3, G2, C3)
        { 45, 2.0f }, { 40, 1.0f }, { 45, 3.0f }, // Am (A2, E2, A2)
        { 46, 2.0f }, { 41, 1.0f }, { 46, 3.0f }, // Bb (Bb2, F2, Bb2)
        { 43, 2.0f }, { 38, 1.0f }, { 43, 3.0f }, // Gm (G2, D2, G2)
        { 48, 2.0f }, { 43, 1.0f }, { 48, 3.0f }, // C  (C3, G2, C3)
        { 45, 2.0f }, { 40, 1.0f }, { 45, 3.0f }  // A7 (A2, E2, A2)
    };
    int bass_count = (int)(sizeof(s_woods_bass) / sizeof(s_woods_bass[0]));

    cur_time = 0.0f;
    for (int b = 0; b < bass_count; b++) {
        float dur_sec = s_woods_bass[b].duration * beat_sec;
        u32 start_frame = (u32)(cur_time * AUDIO_SAMPLE_RATE);
        u32 num_frames = (u32)(dur_sec * AUDIO_SAMPLE_RATE);
        float freq = note_to_freq(s_woods_bass[b].note);
        float phase = 0.0f;

        for (u32 i = 0; i < num_frames; i++) {
            u32 idx = (start_frame + i) % total_frames;
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float env = expf(-2.5f * t);

            phase += freq / (float)AUDIO_SAMPLE_RATE;
            float sample = synth_triangle_wave(phase) * env * 0.35f;

            mix_l[idx] += sample * 0.5f;
            mix_r[idx] += sample * 0.5f;
        }
        cur_time += dur_sec;
    }

    // Canal 4: Shaker de Folhagem e Percussao Organica
    for (int bar = 0; bar < 16; bar++) {
        for (int beat = 1; beat <= 2; beat++) {
            float shaker_time = (bar * 3.0f * beat_sec) + (beat * beat_sec);
            u32 start_frame = (u32)(shaker_time * AUDIO_SAMPLE_RATE);
            u32 num_frames = (u32)(0.06f * AUDIO_SAMPLE_RATE);

            for (u32 i = 0; i < num_frames; i++) {
                u32 idx = (start_frame + i) % total_frames;
                float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                float env = expf(-55.0f * t);
                float sample = synth_noise() * env * 0.07f;
                mix_l[idx] += sample * 0.45f;
                mix_r[idx] += sample * 0.55f;
            }
        }
    }

    s16* out_buf = (s16*)malloc(total_frames * 2 * sizeof(s16));
    if (!out_buf) {
        free(mix_l);
        free(mix_r);
        return NULL;
    }

    for (u32 i = 0; i < total_frames; i++) {
        float l = mix_l[i] * 30000.0f;
        float r = mix_r[i] * 30000.0f;
        if (l > 32767.0f)  l = 32767.0f;
        if (l < -32768.0f) l = -32768.0f;
        if (r > 32767.0f)  r = 32767.0f;
        if (r < -32768.0f) r = -32768.0f;

        out_buf[i * 2 + 0] = (s16)l;
        out_buf[i * 2 + 1] = (s16)r;
    }

    free(mix_l);
    free(mix_r);
    *out_total_frames = total_frames;
    return out_buf;
}

static s16* synth_generate_hyrule_overworld(u32* out_total_frames) {
    float bpm = 144.0f;
    float beat_sec = 60.0f / bpm;
    float total_sec = beat_sec * 4.0f * 12.0f; // 12 compassos em 4/4 = 48 batidas = 20.0s
    u32 total_frames = (u32)(AUDIO_SAMPLE_RATE * total_sec);

    float* mix_l = (float*)calloc(total_frames, sizeof(float));
    float* mix_r = (float*)calloc(total_frames, sizeof(float));
    if (!mix_l || !mix_r) {
        if (mix_l) free(mix_l);
        if (mix_r) free(mix_r);
        return NULL;
    }

    // Canal 1: Fanfarra de Metais / Trompete do Heroi (Onda de Pulso 50% GBA)
    static const NoteEvent s_hyrule_lead[] = {
        // Compasso 1 (Bb)
        { 70, 1.0f }, { 65, 0.75f }, { 70, 0.25f }, { 72, 0.5f }, { 74, 0.5f }, { 75, 0.5f }, { 0, 0.5f },
        // Compasso 2 (F -> Fanfarra)
        { 77, 2.0f }, { 77, 1.0f }, { 77, 0.333f }, { 78, 0.333f }, { 80, 0.334f },
        // Compasso 3 (Bb -> Climax)
        { 82, 2.0f }, { 82, 1.0f }, { 80, 0.5f }, { 78, 0.5f },
        // Compasso 4 (Ab -> F)
        { 80, 1.0f }, { 77, 3.0f },
        // Compasso 5 (Eb -> Gb)
        { 75, 1.0f }, { 75, 0.5f }, { 77, 0.5f }, { 78, 2.0f },
        // Compasso 6 (Passagem heroica)
        { 77, 0.5f }, { 75, 0.5f }, { 73, 0.5f }, { 75, 0.5f }, { 77, 1.0f }, { 73, 1.0f },
        // Compasso 7 (Bb)
        { 70, 3.0f }, { 70, 0.5f }, { 72, 0.5f },
        // Compasso 8 (Marcha ascendente)
        { 74, 1.0f }, { 74, 1.0f }, { 74, 0.5f }, { 75, 0.5f }, { 77, 2.0f },
        // Compasso 9 (Descida triunfal)
        { 75, 0.5f }, { 74, 0.5f }, { 72, 1.0f }, { 75, 1.0f }, { 74, 0.5f }, { 72, 0.5f },
        // Compasso 10 (Cadencia para o tema)
        { 70, 1.0f }, { 72, 1.0f }, { 74, 1.0f }, { 72, 1.0f },
        // Compassos 11 e 12 (Sustentacao triunfal e transicao de loop)
        { 70, 4.0f }, { 0, 4.0f }
    };
    int lead_count = (int)(sizeof(s_hyrule_lead) / sizeof(s_hyrule_lead[0]));

    float cur_time = 0.0f;
    for (int n = 0; n < lead_count; n++) {
        float dur_sec = s_hyrule_lead[n].duration * beat_sec;
        u32 start_frame = (u32)(cur_time * AUDIO_SAMPLE_RATE);
        u32 num_frames = (u32)(dur_sec * AUDIO_SAMPLE_RATE);
        float base_freq = note_to_freq(s_hyrule_lead[n].note);
        float phase = 0.0f;

        if (base_freq > 0.0f) {
            for (u32 i = 0; i < num_frames; i++) {
                u32 idx = (start_frame + i) % total_frames;
                float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                float t_note = t / dur_sec;

                float att = (t < 0.02f) ? (t / 0.02f) : 1.0f;
                float dec = 1.0f - (t_note * 0.18f);
                if (t_note > 0.86f) {
                    dec *= (1.0f - t_note) / 0.14f;
                    if (dec < 0.0f) dec = 0.0f;
                }

                float cur_freq = base_freq;
                if (dur_sec > 0.40f && t > 0.22f) {
                    cur_freq += sinf(2.0f * PI_F * 6.0f * (t - 0.22f)) * (base_freq * 0.018f);
                }

                phase += cur_freq / (float)AUDIO_SAMPLE_RATE;
                float sample = synth_square_wave(phase, 0.50f) * att * dec * 0.38f;

                mix_l[idx] += sample * 0.50f;
                mix_r[idx] += sample * 0.50f;
            }
        }
        cur_time += dur_sec;
    }

    // Canal 2: Acordes de Acompanhamento e Metais Staccato (Onda de 25% Duty)
    static const NoteEvent s_hyrule_chords[] = {
        // Bar 1 (Bb)
        { 62, 0.5f }, { 0, 0.5f }, { 62, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f },
        // Bar 2 (F -> Bb)
        { 65, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f }, { 66, 0.5f }, { 0, 0.5f }, { 68, 0.5f }, { 0, 0.5f },
        // Bar 3 (Gb -> Ab)
        { 70, 0.5f }, { 0, 0.5f }, { 70, 0.5f }, { 0, 0.5f }, { 68, 0.5f }, { 0, 0.5f }, { 66, 0.5f }, { 0, 0.5f },
        // Bar 4 (Ab -> F)
        { 68, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f }, { 65, 1.0f }, { 0, 1.0f },
        // Bar 5 (Ebm -> Gb)
        { 63, 0.5f }, { 0, 0.5f }, { 63, 0.5f }, { 0, 0.5f }, { 66, 0.5f }, { 0, 0.5f }, { 66, 0.5f }, { 0, 0.5f },
        // Bar 6 (Db)
        { 65, 0.5f }, { 0, 0.5f }, { 63, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f }, { 61, 0.5f }, { 0, 0.5f },
        // Bar 7 (Bb)
        { 58, 0.5f }, { 0, 0.5f }, { 58, 0.5f }, { 0, 0.5f }, { 62, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f },
        // Bar 8 (Dm -> F)
        { 62, 0.5f }, { 0, 0.5f }, { 62, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f },
        // Bar 9 (Eb -> F)
        { 63, 0.5f }, { 0, 0.5f }, { 60, 0.5f }, { 0, 0.5f }, { 63, 0.5f }, { 0, 0.5f }, { 65, 0.5f }, { 0, 0.5f },
        // Bar 10 (Bb -> F)
        { 58, 0.5f }, { 0, 0.5f }, { 60, 0.5f }, { 0, 0.5f }, { 62, 0.5f }, { 0, 0.5f }, { 60, 0.5f }, { 0, 0.5f },
        // Bars 11 e 12 (Bb sustentado)
        { 58, 2.0f }, { 62, 2.0f }, { 65, 2.0f }, { 0, 2.0f }
    };
    int chord_count = (int)(sizeof(s_hyrule_chords) / sizeof(s_hyrule_chords[0]));

    cur_time = 0.0f;
    for (int c = 0; c < chord_count; c++) {
        float dur_sec = s_hyrule_chords[c].duration * beat_sec;
        u32 start_frame = (u32)(cur_time * AUDIO_SAMPLE_RATE);
        u32 num_frames = (u32)(dur_sec * AUDIO_SAMPLE_RATE);
        float freq = note_to_freq(s_hyrule_chords[c].note);
        float phase = 0.0f;

        if (freq > 0.0f) {
            for (u32 i = 0; i < num_frames; i++) {
                u32 idx = (start_frame + i) % total_frames;
                float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                float env = expf(-14.0f * t);

                phase += freq / (float)AUDIO_SAMPLE_RATE;
                float sample = synth_square_wave(phase, 0.25f) * env * 0.18f;

                mix_l[idx] += sample * 0.65f;
                mix_r[idx] += sample * 0.35f;
            }
        }
        cur_time += dur_sec;
    }

    // Canal 3: Baixo de Marcha Marcial (Onda Triangular PSG)
    static const NoteEvent s_hyrule_bass[] = {
        // Bar 1: Bb2, Bb2, D3, F3
        { 46, 1.0f }, { 46, 1.0f }, { 50, 1.0f }, { 53, 1.0f },
        // Bar 2: F2, F2, A2, C3
        { 41, 1.0f }, { 41, 1.0f }, { 45, 1.0f }, { 48, 1.0f },
        // Bar 3: Gb2, Gb2, Bb2, Db3
        { 42, 1.0f }, { 42, 1.0f }, { 46, 1.0f }, { 49, 1.0f },
        // Bar 4: Ab2, Ab2, C3, Eb3
        { 44, 1.0f }, { 44, 1.0f }, { 48, 1.0f }, { 51, 1.0f },
        // Bar 5: Eb2, Eb2, G2, Bb2
        { 39, 1.0f }, { 39, 1.0f }, { 43, 1.0f }, { 46, 1.0f },
        // Bar 6: Db2, Db2, F2, Ab2
        { 37, 1.0f }, { 37, 1.0f }, { 41, 1.0f }, { 44, 1.0f },
        // Bar 7: Bb2, Bb2, D3, F3
        { 46, 1.0f }, { 46, 1.0f }, { 50, 1.0f }, { 53, 1.0f },
        // Bar 8: D2, D2, F#2, A2
        { 38, 1.0f }, { 38, 1.0f }, { 42, 1.0f }, { 45, 1.0f },
        // Bar 9: Eb2, Eb2, F2, F2
        { 39, 1.0f }, { 39, 1.0f }, { 41, 1.0f }, { 41, 1.0f },
        // Bar 10: Bb2, D3, F3, F2
        { 46, 1.0f }, { 50, 1.0f }, { 53, 1.0f }, { 41, 1.0f },
        // Bars 11 e 12: Bb2 gran finale
        { 46, 4.0f }, { 46, 4.0f }
    };
    int bass_count = (int)(sizeof(s_hyrule_bass) / sizeof(s_hyrule_bass[0]));

    cur_time = 0.0f;
    for (int b = 0; b < bass_count; b++) {
        float dur_sec = s_hyrule_bass[b].duration * beat_sec;
        u32 start_frame = (u32)(cur_time * AUDIO_SAMPLE_RATE);
        u32 num_frames = (u32)(dur_sec * AUDIO_SAMPLE_RATE);
        float freq = note_to_freq(s_hyrule_bass[b].note);
        float phase = 0.0f;

        for (u32 i = 0; i < num_frames; i++) {
            u32 idx = (start_frame + i) % total_frames;
            float t = (float)i / (float)AUDIO_SAMPLE_RATE;
            float env = expf(-3.2f * t);

            phase += freq / (float)AUDIO_SAMPLE_RATE;
            float sample = synth_triangle_wave(phase) * env * 0.32f;

            mix_l[idx] += sample * 0.5f;
            mix_r[idx] += sample * 0.5f;
        }
        cur_time += dur_sec;
    }

    // Canal 4: Caixa Marcial e Prato de Conducao (Ruido Ritmico de Marcha)
    for (int bar = 0; bar < 12; bar++) {
        float bar_time = bar * 4.0f * beat_sec;

        if (bar == 0 || bar == 6) {
            u32 start_frame = (u32)(bar_time * AUDIO_SAMPLE_RATE);
            u32 num_frames = (u32)(0.35f * AUDIO_SAMPLE_RATE);
            for (u32 i = 0; i < num_frames; i++) {
                u32 idx = (start_frame + i) % total_frames;
                float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                float env = expf(-12.0f * t);
                float sample = synth_noise() * env * 0.15f;
                mix_l[idx] += sample * 0.55f;
                mix_r[idx] += sample * 0.45f;
            }
        }

        for (int beat = 0; beat < 4; beat++) {
            float beat_time = bar_time + beat * beat_sec;

            if (beat == 2) {
                for (int r = 0; r < 4; r++) {
                    u32 start_frame = (u32)((beat_time + r * (beat_sec * 0.25f)) * AUDIO_SAMPLE_RATE);
                    u32 num_frames = (u32)(0.04f * AUDIO_SAMPLE_RATE);
                    for (u32 i = 0; i < num_frames; i++) {
                        u32 idx = (start_frame + i) % total_frames;
                        float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                        float env = expf(-45.0f * t);
                        float sample = synth_noise() * env * 0.10f;
                        mix_l[idx] += sample * 0.48f;
                        mix_r[idx] += sample * 0.52f;
                    }
                }
            } else {
                u32 start_frame = (u32)(beat_time * AUDIO_SAMPLE_RATE);
                u32 num_frames = (u32)(0.09f * AUDIO_SAMPLE_RATE);
                float amp = (beat == 1 || beat == 3) ? 0.16f : 0.11f;

                for (u32 i = 0; i < num_frames; i++) {
                    u32 idx = (start_frame + i) % total_frames;
                    float t = (float)i / (float)AUDIO_SAMPLE_RATE;
                    float env = expf(-38.0f * t);
                    float sample = synth_noise() * env * amp;
                    mix_l[idx] += sample * 0.5f;
                    mix_r[idx] += sample * 0.5f;
                }
            }
        }
    }

    s16* out_buf = (s16*)malloc(total_frames * 2 * sizeof(s16));
    if (!out_buf) {
        free(mix_l);
        free(mix_r);
        return NULL;
    }

    for (u32 i = 0; i < total_frames; i++) {
        float l = mix_l[i] * 29000.0f;
        float r = mix_r[i] * 29000.0f;
        if (l > 32767.0f)  l = 32767.0f;
        if (l < -32768.0f) l = -32768.0f;
        if (r > 32767.0f)  r = 32767.0f;
        if (r < -32768.0f) r = -32768.0f;

        out_buf[i * 2 + 0] = (s16)l;
        out_buf[i * 2 + 1] = (s16)r;
    }

    free(mix_l);
    free(mix_r);
    *out_total_frames = total_frames;
    return out_buf;
}

// ----------------------------------------------------------------------------
// CALLBACK DO MIXER DE BAIXA LATÊNCIA DO SDL2
// ----------------------------------------------------------------------------

static void audio_mixer_callback(void* userdata, Uint8* stream, int len) {
    (void)userdata;
    int num_samples = len / sizeof(s16); // Total de amostras individuais no buffer
    int num_frames  = num_samples / AUDIO_CHANNELS; // Quadros estéreo (L + R)

    s16* out_ptr = (s16*)stream;
    memset(stream, 0, len); // Silêncio base

    // Buffers de acumulação de 32-bit para evitar overflow durante a mixagem
    s32 mix_buffer_l[AUDIO_BUFFER_SIZE];
    s32 mix_buffer_r[AUDIO_BUFFER_SIZE];
    memset(mix_buffer_l, 0, sizeof(mix_buffer_l));
    memset(mix_buffer_r, 0, sizeof(mix_buffer_r));

    // 1. Mixa a trilha sonora de fundo (BGM)
    if (s_bgm_voice.is_active && s_bgm_voice.samples) {
        float gain = s_bgm_voice.volume * s_vol_bgm;
        for (int i = 0; i < num_frames; i++) {
            u32 frame_idx = (u32)s_bgm_voice.current_frame;
            if (frame_idx >= s_bgm_voice.total_frames) {
                if (s_bgm_voice.is_looping) {
                    s_bgm_voice.current_frame = 0.0f;
                    frame_idx = 0;
                } else {
                    s_bgm_voice.is_active = false;
                    break;
                }
            }

            s32 sample_l = 0;
            s32 sample_r = 0;
            if (s_bgm_voice.is_stereo) {
                sample_l = s_bgm_voice.samples[frame_idx * 2 + 0];
                sample_r = s_bgm_voice.samples[frame_idx * 2 + 1];
            } else {
                sample_l = s_bgm_voice.samples[frame_idx];
                sample_r = sample_l;
            }

            mix_buffer_l[i] += (s32)(sample_l * gain);
            mix_buffer_r[i] += (s32)(sample_r * gain);

            s_bgm_voice.current_frame += s_bgm_voice.pitch;
        }
    }

    // 2. Mixa todas as vozes simultâneas de efeitos sonoros (SFX)
    for (int v = 0; v < MAX_CONCURRENT_VOICES; v++) {
        Voice* voice = &s_voices[v];
        if (!voice->is_active || !voice->samples) continue;

        float gain = voice->volume * s_vol_sfx;

        for (int i = 0; i < num_frames; i++) {
            u32 frame_idx = (u32)voice->current_frame;
            if (frame_idx >= voice->total_frames) {
                if (voice->is_looping) {
                    voice->current_frame = 0.0f;
                    frame_idx = 0;
                } else {
                    voice->is_active = false;
                    if (voice->free_on_finish && voice->samples) {
                        free(voice->samples);
                        voice->samples = NULL;
                    }
                    break;
                }
            }

            s32 sample_l = 0;
            s32 sample_r = 0;
            if (voice->is_stereo) {
                sample_l = voice->samples[frame_idx * 2 + 0];
                sample_r = voice->samples[frame_idx * 2 + 1];
            } else {
                sample_l = voice->samples[frame_idx];
                sample_r = sample_l;
            }

            mix_buffer_l[i] += (s32)(sample_l * gain);
            mix_buffer_r[i] += (s32)(sample_r * gain);

            voice->current_frame += voice->pitch;
        }
    }

    // 3. Aplica o Volume Master e proteção contra distorção (Clamping / Saturação)
    for (int i = 0; i < num_frames; i++) {
        s32 final_l = (s32)(mix_buffer_l[i] * s_vol_master);
        s32 final_r = (s32)(mix_buffer_r[i] * s_vol_master);

        if (final_l > 32767)  final_l = 32767;
        if (final_l < -32768) final_l = -32768;
        if (final_r > 32767)  final_r = 32767;
        if (final_r < -32768) final_r = -32768;

        out_ptr[i * 2 + 0] = (s16)final_l;
        out_ptr[i * 2 + 1] = (s16)final_r;
    }
}

// ----------------------------------------------------------------------------
// INTERFACE PÚBLICA DA HAL AUDIO
// ----------------------------------------------------------------------------

bool hal_audio_init(void) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("[AUDIO ERRO] Falha ao inicializar subsistema SDL_Audio: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq     = AUDIO_SAMPLE_RATE;
    desired.format   = AUDIO_S16SYS;
    desired.channels = AUDIO_CHANNELS;
    desired.samples  = AUDIO_BUFFER_SIZE;
    desired.callback = audio_mixer_callback;
    desired.userdata = NULL;

    s_audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &s_audio_spec, 0);
    if (s_audio_device == 0) {
        printf("[AUDIO AVISO] Nenhum dispositivo de som encontrado: %s\n", SDL_GetError());
        return false;
    }

    // Limpa todas as vozes
    memset(s_voices, 0, sizeof(s_voices));
    memset(&s_bgm_voice, 0, sizeof(s_bgm_voice));

    // Gera em tempo de execução os efeitos sonoros do sintetizador procedual retro
    synth_generate_all_sfx();

    // Despausa o mixer do hardware para iniciar a reprodução em tempo real
    SDL_PauseAudioDevice(s_audio_device, 0);
    s_audio_ready = true;

    printf("[HAL Audio] Inicializado com sucesso! (44100Hz, Estereo 16-bit, Latencia: ~%.1fms)\n",
           (float)AUDIO_BUFFER_SIZE * 1000.0f / AUDIO_SAMPLE_RATE);
    return true;
}

void hal_audio_play_sound(SoundEffect effect, float volume, float pitch_shift) {
    if (!s_audio_ready || effect < 0 || effect >= SOUND_COUNT) return;

    PrecalcSfx* sfx = &s_precalc_sfx[effect];
    if (!sfx->samples) return;

    SDL_LockAudioDevice(s_audio_device);

    // Encontra uma voz inativa no mixer
    int voice_idx = -1;
    for (int v = 0; v < MAX_CONCURRENT_VOICES; v++) {
        if (!s_voices[v].is_active) {
            voice_idx = v;
            break;
        }
    }

    // Se todas estiverem ocupadas, reutiliza a voz mais antiga (voz 0)
    if (voice_idx == -1) voice_idx = 0;

    Voice* voice = &s_voices[voice_idx];
    voice->samples        = sfx->samples;
    voice->total_frames   = sfx->total_frames;
    voice->current_frame  = 0.0f;
    voice->volume         = volume;
    voice->pitch          = (pitch_shift > 0.1f) ? pitch_shift : 1.0f;
    voice->is_active      = true;
    voice->is_looping     = false;
    voice->is_stereo      = sfx->is_stereo;
    voice->free_on_finish = false;

    SDL_UnlockAudioDevice(s_audio_device);
}

bool hal_audio_play_music(const char* wav_path, float volume, bool loop) {
    if (!s_audio_ready || !wav_path) return false;

    SDL_AudioSpec wav_spec;
    Uint8* wav_buffer = NULL;
    Uint32 wav_length = 0;

    if (SDL_LoadWAV(wav_path, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        printf("[AUDIO] Arquivo de musica nao encontrado: %s\n", wav_path);
        return false;
    }

    // Converte automaticamente qualquer formato de WAV para o formato nativo do nosso mixer
    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          s_audio_spec.format, s_audio_spec.channels, s_audio_spec.freq) < 0) {
        SDL_FreeWAV(wav_buffer);
        return false;
    }

    cvt.len = wav_length;
    cvt.buf = (Uint8*)malloc(cvt.len * cvt.len_mult);
    memcpy(cvt.buf, wav_buffer, wav_length);
    SDL_FreeWAV(wav_buffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        free(cvt.buf);
        return false;
    }

    SDL_LockAudioDevice(s_audio_device);

    if (s_bgm_voice.free_on_finish && s_bgm_voice.samples) {
        free(s_bgm_voice.samples);
    }

    s_bgm_voice.samples        = (s16*)cvt.buf;
    s_bgm_voice.total_frames   = cvt.len_cvt / (sizeof(s16) * AUDIO_CHANNELS);
    s_bgm_voice.current_frame  = 0.0f;
    s_bgm_voice.volume         = volume;
    s_bgm_voice.pitch          = 1.0f;
    s_bgm_voice.is_active      = true;
    s_bgm_voice.is_looping     = loop;
    s_bgm_voice.is_stereo      = true;
    s_bgm_voice.free_on_finish = true;

    SDL_UnlockAudioDevice(s_audio_device);
    printf("[AUDIO] Reproduzindo BGM: %s\n", wav_path);
    return true;
}

const char* hal_audio_get_bgm_name(BgmTrack track) {
    switch (track) {
        case BGM_MINISH_WOODS:     return "Minish Woods (Deepwood)";
        case BGM_HYRULE_OVERWORLD: return "Hyrule Overworld (Theme)";
        case BGM_NONE:
        default:                   return "Mudo / Silencio";
    }
}

BgmTrack hal_audio_get_current_bgm(void) {
    return s_current_bgm_track;
}

void hal_audio_play_bgm(BgmTrack track) {
    if (!s_audio_ready) return;
    if (track == s_current_bgm_track && s_bgm_voice.is_active) return;

    if (track == BGM_NONE) {
        hal_audio_stop_music();
        printf("[HAL Audio] BGM parada (Silencio).\n");
        return;
    }

    // 1. Suporte a mods de audio: verifica se existe arquivo WAV customizado em assets/audio/
    char mod_path[256];
    const char* track_tags[] = { "none", "minish_woods", "hyrule_overworld" };
    snprintf(mod_path, sizeof(mod_path), "assets/audio/%s.wav", track_tags[track]);

    if (hal_audio_play_music(mod_path, 0.75f, true)) {
        s_current_bgm_track = track;
        printf("[HAL Audio] Reproduzindo BGM personalizada: %s\n", mod_path);
        return;
    }

    // 2. Sintese Procedural Chiptune Autentica do GBA (4 canais emulados)
    u32 total_frames = 0;
    s16* samples = NULL;

    if (track == BGM_MINISH_WOODS) {
        samples = synth_generate_minish_woods(&total_frames);
    } else if (track == BGM_HYRULE_OVERWORLD) {
        samples = synth_generate_hyrule_overworld(&total_frames);
    }

    if (!samples || total_frames == 0) return;

    SDL_LockAudioDevice(s_audio_device);

    if (s_bgm_voice.free_on_finish && s_bgm_voice.samples) {
        free(s_bgm_voice.samples);
    }

    s_bgm_voice.samples        = samples;
    s_bgm_voice.total_frames   = total_frames;
    s_bgm_voice.current_frame  = 0.0f;
    s_bgm_voice.volume         = 0.85f;
    s_bgm_voice.pitch          = 1.0f;
    s_bgm_voice.is_active      = true;
    s_bgm_voice.is_looping     = true;
    s_bgm_voice.is_stereo      = true;
    s_bgm_voice.free_on_finish = true;

    s_current_bgm_track = track;

    SDL_UnlockAudioDevice(s_audio_device);
    printf("[HAL Audio] Trilha sonora chiptune iniciada: %s (%.1fs em loop continuo)\n",
           hal_audio_get_bgm_name(track),
           (float)total_frames / (float)AUDIO_SAMPLE_RATE);
}

void hal_audio_cycle_bgm(void) {
    BgmTrack next = (s_current_bgm_track + 1) % BGM_COUNT;
    hal_audio_play_bgm(next);
}

void hal_audio_stop_music(void) {
    if (!s_audio_ready) return;

    SDL_LockAudioDevice(s_audio_device);
    s_bgm_voice.is_active = false;
    if (s_bgm_voice.free_on_finish && s_bgm_voice.samples) {
        free(s_bgm_voice.samples);
        s_bgm_voice.samples = NULL;
    }
    s_current_bgm_track = BGM_NONE;
    SDL_UnlockAudioDevice(s_audio_device);
}

void hal_audio_set_master_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    s_vol_master = volume;
}

void hal_audio_set_sfx_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    s_vol_sfx = volume;
}

void hal_audio_set_bgm_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    s_vol_bgm = volume;
}

float hal_audio_get_master_volume(void) { return s_vol_master; }
float hal_audio_get_sfx_volume(void)    { return s_vol_sfx; }
float hal_audio_get_bgm_volume(void)    { return s_vol_bgm; }

void hal_audio_shutdown(void) {
    if (s_audio_device != 0) {
        SDL_CloseAudioDevice(s_audio_device);
        s_audio_device = 0;
    }

    for (int i = 0; i < SOUND_COUNT; i++) {
        if (s_precalc_sfx[i].samples) {
            free(s_precalc_sfx[i].samples);
            s_precalc_sfx[i].samples = NULL;
        }
    }

    if (s_bgm_voice.free_on_finish && s_bgm_voice.samples) {
        free(s_bgm_voice.samples);
        s_bgm_voice.samples = NULL;
    }

    s_audio_ready = false;
    printf("[HAL Audio] Subsistema de som finalizado.\n");
}
