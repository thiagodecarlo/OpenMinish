#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

/*
 * ============================================================================
 * include/hal/audio.h - Camada de Abstração de Áudio (HAL Audio & Mixer)
 * ============================================================================
 * Modela o hardware de som do Game Boy Advance (DirectSound PCM + PSG de 4 canais)
 * utilizando o pipeline de áudio de baixa latência do SDL2.
 *
 * Recursos:
 *   - Mixer multicanal com proteção contra clipping digital por saturação
 *   - Sintetizador procedual retro integrado (emula ondas quadradas PSG e ruído)
 *   - Efeitos sonoros clássicos de Zelda (espada, passos, rolar, segredo)
 *   - Suporte a reprodução de arquivos WAV para trilhas sonoras e mods
 *   - Controle independente de volume Master, SFX e BGM
 */

#include "gba/types.h"
#include <stdbool.h>

// Identificadores de Efeitos Sonoros (SFX)
typedef enum {
    SOUND_SWORD_SLASH = 0, // Golpe de espada do Link
    SOUND_SWORD_HIT,       // Espada atingindo objeto sólido
    SOUND_FOOTSTEP,        // Som de passo na grama/pedra
    SOUND_ROLL,            // Rolar / Esquiva ágil
    SOUND_HEART_BEEP,      // Chime de vida baixa
    SOUND_SECRET,          // O lendário acorde de segredo revelado (8 notas de Zelda)
    SOUND_COUNT
} SoundEffect;

/*
 * Inicializa o subsistema de áudio, configura o dispositivo de saída (44100Hz 16-bit Stereo)
 * e pré-gera as tabelas do sintetizador procedual.
 */
bool hal_audio_init(void);

/*
 * Toca um efeito sonoro retro específico usando o mixer interno.
 * volume: multiplicador de 0.0f (mudo) a 1.0f (máximo).
 * pitch_shift: multiplicador de tom (1.0f = padrão, 1.2f = agudo, 0.8f = grave).
 */
void hal_audio_play_sound(SoundEffect effect, float volume, float pitch_shift);

/*
 * Carrega e reproduz uma música ou amostra sonora a partir de um arquivo WAV.
 * Se loop for true, a música reinicia infinitamente até ser parada.
 */
bool hal_audio_play_music(const char* wav_path, float volume, bool loop);

/*
 * Para imediatamente a música de fundo em execução.
 */
void hal_audio_stop_music(void);

/*
 * Ajusta os níveis de volume globais (0.0f a 1.0f).
 */
void hal_audio_set_master_volume(float volume);
void hal_audio_set_sfx_volume(float volume);
void hal_audio_set_bgm_volume(float volume);

float hal_audio_get_master_volume(void);
float hal_audio_get_sfx_volume(void);
float hal_audio_get_bgm_volume(void);

/*
 * Finaliza o dispositivo de áudio e libera recursos de memória.
 */
void hal_audio_shutdown(void);

#endif // HAL_AUDIO_H
