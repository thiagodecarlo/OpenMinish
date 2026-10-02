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

// ----------------------------------------------------------------------------
// SINTETIZADOR PROCEDURAL RETRO (EMULAÇÃO DE PSG E DIRECTSOUND)
// ----------------------------------------------------------------------------

static float synth_square_wave(float phase, float duty) {
    float p = fmodf(phase, 1.0f);
    if (p < 0.0f) p += 1.0f;
    return (p < duty) ? 1.0f : -1.0f;
}

static float synth_noise(void) {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

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

void hal_audio_stop_music(void) {
    if (!s_audio_ready) return;

    SDL_LockAudioDevice(s_audio_device);
    s_bgm_voice.is_active = false;
    if (s_bgm_voice.free_on_finish && s_bgm_voice.samples) {
        free(s_bgm_voice.samples);
        s_bgm_voice.samples = NULL;
    }
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
