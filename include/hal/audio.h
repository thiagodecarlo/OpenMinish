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
    SOUND_TEXT_BLIP,       // Som curto retro de caractere do diálogo (typewriter)
    SOUND_TEXT_ADVANCE,    // Som de avanço de página ou encerramento do diálogo
    SOUND_EZLO_ALERT,      // Piado característico do pássaro Ezlo ao ser chamado
    SOUND_KEESE_CHIRP,     // Rufal de asas e guincho do morcego Keese ao alçar voo
    SOUND_CHUCHU_SQUISH,   // Som elástico/gelatinoso de salto e brotar do ChuChu
    SOUND_BOOMERANG_FLY,   // Zunido rotativo do bumerangue voando no ar
    SOUND_GUST_SUCTION,    // Vórtice contínuo de sucção de vento do Pote Mágico (Gust Jar)
    SOUND_GUST_BLAST,      // Disparo de projétil de ar pressurizado do Pote Mágico
    SOUND_ITEM_CATCH,      // Captura do bumerangue de volta nas mãos do herói
    SOUND_KINSTONE_FUSION, // Fanfarra mágica de fusão bem-sucedida de Kinstones (arpejo ascendente de harpa)
    SOUND_KINSTONE_PROMPT, // Chime sonoro de balão de Kinstone sobre o NPC
    SOUND_CHEST_OPEN,      // Abertura do baú dourado destravado pela fusão
    SOUND_SWITCH_CLICK,    // Clique mecânico de interruptor de piso afundando na pedra
    SOUND_DOOR_UNLOCK,     // Cadeado de ferro se abrindo com giro de chave
    SOUND_DOOR_SHUTTER,    // Grades de ferro da masmorra se erguendo ou baixando
    SOUND_BLOCK_PUSH,      // Bloco pesado de pedra deslizando pelo piso
    SOUND_BOSS_SLAM,       // Impacto colossal do chefe desabando no chão
    SOUND_BOSS_HIT,        // Golpe certeiro na cabeça gelatinosa vulnerável do chefe
    SOUND_BOSS_DEFEAT,     // Explosão climática de vitória e dissolução do chefe
    SOUND_HEART_CONTAINER, // Fanfarra sagrada ao obter o Heart Container permanente
    SOUND_SPIN_CHARGE,     // Zunido crescente de carga de energia da espada
    SOUND_SPIN_READY,      // Chime metálico agudo quando a carga do Spin Attack atinge o máximo
    SOUND_SPIN_ATTACK,     // Giro veloz cortante da lâmina em 360 graus
    SOUND_TIGER_SCROLL,    // Fanfarra marcial sagrada ao receber o Pergaminho do Tigre (Tiger Scroll)
    SOUND_COUNT
} SoundEffect;

// Identificadores de Trilhas Sonoras (BGM)
typedef enum {
    BGM_NONE = 0,
    BGM_MINISH_WOODS,     // Trilha misteriosa e mágica de Minish Woods (Deepwood)
    BGM_HYRULE_OVERWORLD, // O lendário tema marcial de Hyrule Field
    BGM_DEEPWOOD_SHRINE,  // A atmosfera enigmática e gótica de Deepwood Shrine
    BGM_BOSS_BATTLE,      // Batalha urgente e épica contra o Chefe (Boss Battle)
    BGM_COUNT
} BgmTrack;

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
 * Inicia a reprodução contínua da trilha sonora (BGM) especificada.
 * Sintetiza o arranjo orquestrado no estilo chiptune autêntico do GBA (4 canais:
 * Lead, Arpeggios/Chords, Bass e Percussão) ou reproduz arquivo customizado se presente em assets/audio/.
 */
void hal_audio_play_bgm(BgmTrack track);

/*
 * Retorna qual trilha sonora está em execução no momento.
 */
BgmTrack hal_audio_get_current_bgm(void);

/*
 * Retorna o nome amigável da trilha sonora.
 */
const char* hal_audio_get_bgm_name(BgmTrack track);

/*
 * Alterna para a próxima trilha sonora disponível (Minish Woods -> Hyrule -> Mudo).
 */
void hal_audio_cycle_bgm(void);

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
