#include <stdio.h>
#include <SDL.h>
#include "gba/types.h"
#include "hal/video.h"
#include "hal/texture.h"
#include "hal/input.h"
#include "hal/audio.h"
#include "hal/map.h"
#include "hal/entity.h"
#include "hal/font.h"
#include "hal/dialogue.h"
#include "hal/subweapon.h"
#include <math.h>

/*
 * ============================================================================
 * PROJETO ACADÊMICO: The Legend of Zelda: The Minish Cap - Native PC Port
 * Arquivo: src/main.c (Ponto de Entrada, Game Loop, Input e Entidade do Herói)
 * ============================================================================
 */

typedef enum {
    REGION_USA = 0,
    REGION_EUR = 1,
    REGION_JPN = 2,
    REGION_COUNT = 3
} SelectedRegion;

typedef struct {
    float x;
    float y;
    float speed;
    Direction dir;
    bool is_moving;
    bool is_attacking;
    int  attack_timer;
    int  anim_timer;
    int  anim_frame;
    int  hearts;
    int  rupees;
    int  invuln_timer;
    float knock_x;
    float knock_y;
} Player;

static const char* s_region_tags[REGION_COUNT] = { "usa", "eur", "jpn" };
static const char* s_region_names[REGION_COUNT] = {
    "USA (Ingles) [BZME]",
    "EUR (Multi-5) [BZMP]",
    "JPN (Japao)   [BZMJ]"
};

static Texture* s_sheet0 = NULL;
static Texture* s_sheet1 = NULL;
static Texture* s_link_tex = NULL;
static Texture* s_octo_tex = NULL;
static Tilemap* s_world_map = NULL;
static SelectedRegion s_current_region = REGION_USA;

static void load_region_sheets(SelectedRegion region) {
    if (s_sheet0)   { texture_free(s_sheet0);   s_sheet0 = NULL; }
    if (s_sheet1)   { texture_free(s_sheet1);   s_sheet1 = NULL; }
    if (s_link_tex) { texture_free(s_link_tex); s_link_tex = NULL; }
    if (s_octo_tex) { texture_free(s_octo_tex); s_octo_tex = NULL; }

    s_current_region = region;

    char path0[256];
    char path1[256];
    char path_link[256];
    char path_octo[256];
    snprintf(path0, sizeof(path0), "assets/regions/%s/sheet_00.bmp", s_region_tags[region]);
    snprintf(path1, sizeof(path1), "assets/regions/%s/sheet_01.bmp", s_region_tags[region]);
    snprintf(path_link, sizeof(path_link), "assets/regions/%s/link.bmp", s_region_tags[region]);
    snprintf(path_octo, sizeof(path_octo), "assets/regions/%s/octorok.bmp", s_region_tags[region]);

    s_sheet0 = texture_load_bmp(path0);
    s_sheet1 = texture_load_bmp(path1);
    s_link_tex = texture_load_bmp(path_link);
    s_octo_tex = texture_load_bmp(path_octo);

    entity_set_texture(s_octo_tex);

    if (s_world_map) {
        map_set_region(s_world_map, s_region_tags[region]);
    }

    printf("[REGIAO ATUALIZADA] -> %s (Link: %s, Octorok: %s, Mapa: %s)\n",
           s_region_names[region],
           s_link_tex ? "Autentico GBA" : "Procedural",
           s_octo_tex ? "Autentico GBA" : "Procedural",
           (s_world_map && s_world_map->is_authentic) ? "Autentico Minish Woods" : "Procedural");
}

static void draw_rect(int rx, int ry, int rw, int rh, u32 color) {
    for (int y = ry; y < ry + rh; y++) {
        for (int x = rx; x < rx + rw; x++) {
            hal_video_put_pixel(x, y, color);
        }
    }
}

// Renderiza um coração clássico de vida de Zelda em pixel art 7x7
static void draw_heart(int hx, int hy) {
    u32 red   = 0xE62222FF;
    u32 dark  = 0x660808FF;
    u32 white = 0xFFFFFFFF;

    // Linha 0: dois topos
    hal_video_put_pixel(hx + 1, hy, red);
    hal_video_put_pixel(hx + 2, hy, red);
    hal_video_put_pixel(hx + 4, hy, red);
    hal_video_put_pixel(hx + 5, hy, red);

    // Linhas 1 a 3: corpo do coração com brilho
    for (int y = 1; y <= 3; y++) {
        for (int x = 0; x <= 6; x++) {
            hal_video_put_pixel(hx + x, hy + y, red);
        }
    }
    hal_video_put_pixel(hx + 1, hy + 1, white); // Brilho

    // Ponta inferior em V
    for (int x = 1; x <= 5; x++) hal_video_put_pixel(hx + x, hy + 4, red);
    for (int x = 2; x <= 4; x++) hal_video_put_pixel(hx + x, hy + 5, red);
    hal_video_put_pixel(hx + 3, hy + 6, red);
}

// Renderiza o Link no estilo clássico de Minish Cap na posição da Câmera
static void draw_link(const Player* p, const Camera* cam) {
    // Efeito clássico de piscar ao receber dano (flicker)
    if (p->invuln_timer > 0 && ((p->invuln_timer / 3) % 2 == 0)) {
        return;
    }

    int px, py;
    map_world_to_screen(cam, p->x, p->y, &px, &py);

    // ------------------------------------------------------------------------
    // RENDERIZADOR AUTÊNTICO COM SPRITES EXTRAÍDOS DA ROM
    // ------------------------------------------------------------------------
    if (s_link_tex && s_link_tex->pixels) {
        int f_idx = 0;
        bool flip_h = false;

        // Animação com alternância de passos (Down: 0/4, Right: 1/5, Up: 2/6)
        if (p->dir == DIR_DOWN) {
            f_idx = (p->is_moving && p->anim_frame == 1) ? 4 : 0;
        } else if (p->dir == DIR_UP) {
            f_idx = (p->is_moving && p->anim_frame == 1) ? 6 : 2;
        } else if (p->dir == DIR_RIGHT) {
            f_idx = (p->is_moving && p->anim_frame == 1) ? 5 : 1;
        } else if (p->dir == DIR_LEFT) {
            f_idx = (p->is_moving && p->anim_frame == 1) ? 5 : 1;
            flip_h = true;
        }

        int src_x = (f_idx % 8) * 16;
        int src_y = (f_idx / 8) * 24;

        // O sprite canônico de 16x24 tem a ponta do gorro em y=0 e as botas em y=23.
        // Como a caixa de colisão do Link fica na base, desenhamos com offset py - 6.
        int draw_y = py - 6;
        if (p->is_moving && p->anim_frame == 1) {
            draw_y -= 1; // Sutil bobbing de caminhada clássica
        }

        texture_draw_ex(s_link_tex, src_x, src_y, 16, 24, px, draw_y, flip_h);

        // Se atacando com a espada, desenha o corte de lâmina
        if (p->is_attacking) {
            u32 sword_steel = 0xCFE2F3FF;
            u32 sword_gold  = 0xFFD700FF;
            switch (p->dir) {
                case DIR_DOWN:
                    draw_rect(px + 6, py + 16, 4, 10, sword_steel);
                    draw_rect(px + 4, py + 16, 8, 2, sword_gold);
                    break;
                case DIR_UP:
                    draw_rect(px + 6, py - 8, 4, 10, sword_steel);
                    draw_rect(px + 4, py + 0, 8, 2, sword_gold);
                    break;
                case DIR_LEFT:
                    draw_rect(px - 10, py + 9, 10, 4, sword_steel);
                    draw_rect(px + 0,  py + 7, 2, 8, sword_gold);
                    break;
                case DIR_RIGHT:
                    draw_rect(px + 14, py + 9, 10, 4, sword_steel);
                    draw_rect(px + 13, py + 7, 2, 8, sword_gold);
                    break;
            }
        }
        return;
    }

    // ------------------------------------------------------------------------
    // FALLBACK PROCEDURAL (utilizado caso os assets não estejam extraídos)
    // ------------------------------------------------------------------------
    u32 tunic_green = 0x228B22FF; // Verde Floresta
    u32 hat_bright   = 0x32CD32FF; // Verde Gorro
    u32 skin_tone    = 0xF5CBA7FF; // Tom de Pele
    u32 belt_brown   = 0x8B4513FF; // Cinto Marrom
    u32 boot_color   = 0xD2691EFF; // Botas
    u32 sword_steel  = 0xCFE2F3FF; // Aço da Espada
    u32 sword_gold   = 0xFFD700FF; // Empunhadura

    int step_offset = (p->is_moving && (p->anim_frame == 1)) ? 1 : 0;

    // 1. Gorro e Cabelo (topo da cabeça)
    draw_rect(px + 3, py + 0, 10, 3, hat_bright);
    draw_rect(px + 2, py + 3, 12, 3, hat_bright);

    // Ponta do Gorro Minish pendendo conforme a direção
    if (p->dir == DIR_LEFT)  draw_rect(px + 12, py + 2, 3, 4, hat_bright);
    if (p->dir == DIR_RIGHT) draw_rect(px + 1,  py + 2, 3, 4, hat_bright);

    // 2. Rosto
    draw_rect(px + 4, py + 6, 8, 4, skin_tone);
    if (p->dir == DIR_DOWN) {
        // Olhos olhando para frente
        hal_video_put_pixel(px + 5, py + 7, 0x111111FF);
        hal_video_put_pixel(px + 9, py + 7, 0x111111FF);
    } else if (p->dir == DIR_LEFT) {
        hal_video_put_pixel(px + 4, py + 7, 0x111111FF);
    } else if (p->dir == DIR_RIGHT) {
        hal_video_put_pixel(px + 10, py + 7, 0x111111FF);
    }

    // 3. Túnica Verde
    draw_rect(px + 3, py + 10, 10, 5, tunic_green);

    // 4. Cinto
    draw_rect(px + 4, py + 13, 8, 2, belt_brown);
    hal_video_put_pixel(px + 7, py + 13, sword_gold); // Fivela dourada

    // 5. Pernas e Botas animadas pelo ciclo de caminhada
    if (step_offset == 0) {
        draw_rect(px + 4, py + 15, 3, 3, boot_color);
        draw_rect(px + 9, py + 15, 3, 3, boot_color);
    } else {
        draw_rect(px + 3, py + 14, 3, 4, boot_color);
        draw_rect(px + 10, py + 15, 3, 3, boot_color);
    }

    // 6. Animação de Golpe de Espada (quando o botão A é pressionado)
    if (p->is_attacking) {
        switch (p->dir) {
            case DIR_DOWN:
                draw_rect(px + 6, py + 16, 4, 10, sword_steel);
                draw_rect(px + 4, py + 16, 8, 2, sword_gold);
                break;
            case DIR_UP:
                draw_rect(px + 6, py - 8, 4, 10, sword_steel);
                draw_rect(px + 4, py + 0, 8, 2, sword_gold);
                break;
            case DIR_LEFT:
                draw_rect(px - 10, py + 9, 10, 4, sword_steel);
                draw_rect(px + 0,  py + 7, 2, 8, sword_gold);
                break;
            case DIR_RIGHT:
                draw_rect(px + 14, py + 9, 10, 4, sword_steel);
                draw_rect(px + 13, py + 7, 2, 8, sword_gold);
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("====================================================================\n");
    printf("   The Legend of Zelda: The Minish Cap - Native PC Port             \n");
    printf("   Camada de Input & Entidade Controlavel do Heroi (Link)           \n");
    printf("====================================================================\n");
    printf("Controles Disponiveis:\n");
    printf("  - Mover Link:     [WASD] ou [Setas do Teclado] ou [D-Pad/Analogico]\n");
    printf("  - Atacar / Acao:  [Z] ou [Espaco] ou [Botao A do Gamepad] (Atacar espada / Falar com NPCs!)\n");
    printf("  - Item Secundar.: [X] ou [Botao B do Gamepad] (Bumerangue / Pote Magico / Pegasus Boots!)\n");
    printf("  - Ciclar Itens:   [Q] ou [Gatilho L no Gamepad] (Alternar item secundario equipado)\n");
    printf("  - Falar com Ezlo: [E] ou [Select no Gamepad] (Dicas e orientacoes do gorro companheiro!)\n");
    printf("  - Trilha Sonora:  [T] ou [Gatilho R no Gamepad] (Minish Woods / Hyrule / Mudo)\n");
    printf("  - Segredo Zelda:  [M] (Chime lendario de 8 notas!)\n");
    printf("  - Alarme de Vida: [H] (Chime classico de coracao)\n");
    printf("  - Trocar Regiao:  [1] USA | [2] EUR | [3] JPN\n");
    printf("  - Widescreen:     [W] Alternar proporcao 16:9\n");
    printf("  - Sair do Jogo:   [ESC]\n\n");

    int scale = 4;
    bool widescreen = false;

    if (!hal_video_init("The Legend of Zelda: The Minish Cap (Port Nativo)", scale, widescreen)) {
        return 1;
    }

    hal_input_init();
    hal_audio_init();
    font_init();
    dialogue_init();
    subweapon_init();

    // Inicia a trilha sonora autêntica de Minish Woods no mixer chiptune da HAL
    hal_audio_play_bgm(BGM_MINISH_WOODS);

    // Inicialização da Engine de Mapas e Câmera Widescreen (Autêntico Minish Woods ou Fallback)
    Tilemap* world_map = map_create_woods(s_region_tags[REGION_USA]);
    s_world_map = world_map;
    load_region_sheets(REGION_USA);

    // Inicialização da entidade do Link
    Player link;
    if (world_map && world_map->is_authentic) {
        // Clareira ensolarada do santuário em Minish Woods (tx = 28, ty = 39)
        link.x = 448.0f;
        link.y = 624.0f;
    } else {
        link.x = 296.0f;
        link.y = 176.0f;
    }
    link.speed = 1.05f; // Calibrado com a velocidade autêntica do GBA (1.0 pixel/frame)
    link.dir = DIR_DOWN;
    link.is_moving = false;
    link.is_attacking = false;
    link.attack_timer = 0;
    link.anim_timer = 0;
    link.anim_frame = 0;
    link.hearts = 3;
    link.rupees = 50;
    link.invuln_timer = 0;
    link.knock_x = 0.0f;
    link.knock_y = 0.0f;

    // Inicialização do Subsistema de Entidades e Spawn de Inimigos e NPCs
    entity_manager_init();
    if (world_map && world_map->is_authentic) {
        // Inimigos clássicos Octorok
        entity_spawn(ENTITY_ENEMY_OCTOROK, 400.0f, 620.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 500.0f, 620.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 448.0f, 500.0f);
        // Morcegos voadores Keese (com sombra e vôo senoidal)
        entity_spawn(ENTITY_ENEMY_KEESE, 380.0f, 540.0f);
        entity_spawn(ENTITY_ENEMY_KEESE, 520.0f, 550.0f);
        // Gosmas Green ChuChu (camufladas no chão)
        entity_spawn(ENTITY_ENEMY_CHUCHU, 420.0f, 660.0f);
        entity_spawn(ENTITY_ENEMY_CHUCHU, 480.0f, 660.0f);
        // Habitante Minish amigável próximo ao caminho da clareira
        entity_spawn(ENTITY_NPC_FOREST_MINISH, 448.0f, 570.0f);
    } else {
        entity_spawn(ENTITY_ENEMY_OCTOROK, 160.0f, 220.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 420.0f, 150.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 340.0f, 310.0f);
        entity_spawn(ENTITY_ENEMY_KEESE, 200.0f, 130.0f);
        entity_spawn(ENTITY_ENEMY_CHUCHU, 320.0f, 220.0f);
        entity_spawn(ENTITY_NPC_FOREST_MINISH, 250.0f, 176.0f);
    }

    Camera camera;
    camera.viewport_w = widescreen ? 284 : 240;
    camera.viewport_h = 160;
    camera.x = link.x - ((float)camera.viewport_w / 2.0f);
    camera.y = link.y - ((float)camera.viewport_h / 2.0f);

    bool running = true;
    SDL_Event event;

    // ========================================================================
    // O GAME LOOP MULTIPLATAFORMA A 60 FPS
    // ========================================================================
    while (running) {
        // --------------------------------------------------------------------
        // 1. CAPTURA DE EVENTOS DO SISTEMA OPERACIONAL
        // --------------------------------------------------------------------
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else {
                hal_input_process_event(&event);

                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_1:
                            load_region_sheets(REGION_USA);
                            break;
                        case SDLK_2:
                            load_region_sheets(REGION_EUR);
                            break;
                        case SDLK_3:
                            load_region_sheets(REGION_JPN);
                            break;
                        case SDLK_w:
                            widescreen = !widescreen;
                            hal_video_shutdown();
                            hal_video_init("The Legend of Zelda: The Minish Cap (Port Nativo)", scale, widescreen);
                            break;
                        case SDLK_m:
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                            break;
                        case SDLK_h:
                            hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);
                            break;
                        case SDLK_t:
                            hal_audio_cycle_bgm();
                            break;
                        case SDLK_e:
                            if (!dialogue_is_active()) {
                                dialogue_trigger_ezlo_hint();
                            }
                            break;
                        case SDLK_q:
                            subweapon_cycle();
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // Atualiza os estados de transição (borda de subida/descida dos botões)
        hal_input_update();

        // --------------------------------------------------------------------
        // 2. ATUALIZAÇÃO DA LÓGICA DO JOGADOR (INPUT -> FÍSICA)
        // --------------------------------------------------------------------
        const HalVideoContext* ctx = hal_video_get_context();

        if (dialogue_is_active()) {
            dialogue_update();
            if (hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_START)) {
                dialogue_advance();
            }
            if (hal_input_is_held(KEY_B)) {
                dialogue_fast_forward();
            }
            link.is_moving = false;
        } else {
            // Ação de Ataque ou Conversa com NPC (Botão A pressionado no frame exato)
            if (hal_input_is_pressed(KEY_A) && !link.is_attacking) {
                Entity* nearby_npc = entity_find_nearby_npc(link.x, link.y, 28.0f);
                if (nearby_npc) {
                    dialogue_trigger_minish_talk();
                } else {
                    link.is_attacking = true;
                    link.attack_timer = 12; // Dura 12 frames (0.2 segundos)
                    hal_audio_play_sound(SOUND_SWORD_SLASH, 1.0f, 1.0f);
                }
            }

            if (link.is_attacking) {
                // Hitbox do golpe de espada dependendo da orientação do Link
                float hit_x = link.x + 4.0f;
                float hit_y = link.y + 4.0f;
                float hit_w = 12.0f;
                float hit_h = 12.0f;

                if (link.dir == DIR_DOWN)  { hit_x = link.x + 1.0f;  hit_y = link.y + 14.0f; hit_w = 14.0f; hit_h = 12.0f; }
                if (link.dir == DIR_UP)    { hit_x = link.x + 1.0f;  hit_y = link.y - 10.0f; hit_w = 14.0f; hit_h = 12.0f; }
                if (link.dir == DIR_LEFT)  { hit_x = link.x - 12.0f; hit_y = link.y + 2.0f;  hit_w = 12.0f; hit_h = 14.0f; }
                if (link.dir == DIR_RIGHT) { hit_x = link.x + 14.0f; hit_y = link.y + 2.0f;  hit_w = 12.0f; hit_h = 14.0f; }

                // Checa acerto contra inimigos (Octoroks) e projéteis (pedras cuspidas)
                entity_check_sword_hit(hit_x, hit_y, hit_w, hit_h, 1, link.dir);

                // Interação da espada com o cenário (cortar arbustos ou abrir baú)
                if (map_interact_slash(world_map, hit_x + 6.0f, hit_y + 6.0f)) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.25f);
                    link.rupees += 5; // Recompensa clássica de Zelda!
                }

                link.attack_timer--;
                if (link.attack_timer <= 0) {
                    link.is_attacking = false;
                }
            }

            // Subsistema de Armas Secundarias e Itens Equipaveis (Botao B)
            if (hal_input_is_pressed(KEY_B)) {
                if (subweapon_get_current() == ITEM_PEGASUS_BOOTS) {
                    hal_audio_play_sound(SOUND_ROLL, 0.70f, 1.0f);
                }
                subweapon_use_pressed(link.x, link.y, link.dir);
            }
            if (hal_input_is_held(KEY_B)) {
                subweapon_use_held(link.x, link.y, link.dir);
            }
            if (hal_input_is_released(KEY_B)) {
                subweapon_use_released(link.x, link.y, link.dir);
            }

            // Chamado de Orientação do Companheiro Ezlo
            if (hal_input_is_pressed(KEY_SELECT)) {
                dialogue_trigger_ezlo_hint();
            }
            if (hal_input_is_pressed(KEY_L)) {
                subweapon_cycle();
            }
            if (hal_input_is_pressed(KEY_R)) {
                hal_audio_cycle_bgm();
            }

        // Modificador de Velocidade: Dash / Pegasus Boots (Botao B segurado)
        float dash_mult = 1.0f;
        if (hal_input_is_held(KEY_B) && subweapon_get_current() == ITEM_PEGASUS_BOOTS) {
            dash_mult = 1.85f; // Arrancada veloz das Botas de Pegasus!
        }

        link.is_moving = false;
        float actual_speed = 0.0f;
        float move_x = 0.0f;
        float move_y = 0.0f;

        if (subweapon_is_gust_active()) {
            // Durante a succao do Pote Magico, o heroi ancora no chao mas pode mirar o cone de vento
            AnalogStick stick = hal_input_get_left_stick();
            if (stick.magnitude > 0.15f) {
                if (fabsf(stick.x) > fabsf(stick.y)) {
                    link.dir = (stick.x > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    link.dir = (stick.y > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                if (hal_input_is_held(KEY_LEFT))  link.dir = DIR_LEFT;
                if (hal_input_is_held(KEY_RIGHT)) link.dir = DIR_RIGHT;
                if (hal_input_is_held(KEY_UP))    link.dir = DIR_UP;
                if (hal_input_is_held(KEY_DOWN))  link.dir = DIR_DOWN;
            }
            subweapon_use_held(link.x, link.y, link.dir);
            link.is_moving = false;
        } else if (!link.is_attacking) {
            AnalogStick stick = hal_input_get_left_stick();

            if (stick.magnitude > 0.08f) {
                // ============================================================
                // MODO ANALÓGICO PROPORCIONAL: 360° com aceleração gradual!
                // ============================================================
                actual_speed = link.speed * stick.magnitude * dash_mult;
                move_x = stick.x * actual_speed;
                move_y = stick.y * actual_speed;
                link.is_moving = true;

                // Define a orientação do sprite pelo eixo de maior inclinação
                if (fabsf(stick.x) > fabsf(stick.y)) {
                    link.dir = (stick.x > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    link.dir = (stick.y > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                // ============================================================
                // MODO DIGITAL: D-Pad da Cruz ou Teclado (WASD / Setas)
                // ============================================================
                float dx = 0.0f;
                float dy = 0.0f;

                if (hal_input_is_held(KEY_LEFT))  { dx -= 1.0f; link.dir = DIR_LEFT; }
                if (hal_input_is_held(KEY_RIGHT)) { dx += 1.0f; link.dir = DIR_RIGHT; }
                if (hal_input_is_held(KEY_UP))    { dy -= 1.0f; link.dir = DIR_UP; }
                if (hal_input_is_held(KEY_DOWN))  { dy += 1.0f; link.dir = DIR_DOWN; }

                if (dx != 0.0f || dy != 0.0f) {
                    // Normalização diagonal (1 / sqrt(2) ≈ 0.7071)
                    if (dx != 0.0f && dy != 0.0f) {
                        dx *= 0.7071f;
                        dy *= 0.7071f;
                    }
                    actual_speed = link.speed * dash_mult;
                    move_x = dx * actual_speed;
                    move_y = dy * actual_speed;
                    link.is_moving = true;
                }
            }
        }

        // --------------------------------------------------------------------
        // SISTEMA DE COLISÃO COM O MAPA (DESLIZAMENTO SUAVE EM X E Y)
        // --------------------------------------------------------------------
        if (link.is_moving) {
            float new_x = link.x + move_x;
            float new_y = link.y + move_y;

            // Hitbox dos pés do Link (largura: x+4 a x+12, altura: y+12 a y+16)
            bool blocked_x = map_is_solid(world_map, new_x + 4.0f, link.y + 12.0f) ||
                             map_is_solid(world_map, new_x + 12.0f, link.y + 12.0f) ||
                             map_is_solid(world_map, new_x + 4.0f, link.y + 16.0f) ||
                             map_is_solid(world_map, new_x + 12.0f, link.y + 16.0f);
            if (!blocked_x) {
                link.x = new_x;
            }

            bool blocked_y = map_is_solid(world_map, link.x + 4.0f, new_y + 12.0f) ||
                             map_is_solid(world_map, link.x + 12.0f, new_y + 12.0f) ||
                             map_is_solid(world_map, link.x + 4.0f, new_y + 16.0f) ||
                             map_is_solid(world_map, link.x + 12.0f, new_y + 16.0f);
            if (!blocked_y) {
                link.y = new_y;
            }

            // Confinamento dentro dos limites do mundo (640x480 pixels)
            if (link.x < 16.0f) link.x = 16.0f;
            if (link.x > (world_map->width * TILE_SIZE) - 32.0f) link.x = (world_map->width * TILE_SIZE) - 32.0f;
            if (link.y < 16.0f) link.y = 16.0f;
            if (link.y > (world_map->height * TILE_SIZE) - 32.0f) link.y = (world_map->height * TILE_SIZE) - 32.0f;
        }

        // --------------------------------------------------------------------
        // FÍSICA DE KNOCKBACK DO HERÓI (RECÚO AO RECEBER DANO)
        // --------------------------------------------------------------------
        if (fabsf(link.knock_x) > 0.05f || fabsf(link.knock_y) > 0.05f) {
            float k_new_x = link.x + link.knock_x;
            float k_new_y = link.y + link.knock_y;

            if (!map_is_solid(world_map, k_new_x + 4.0f, link.y + 12.0f) &&
                !map_is_solid(world_map, k_new_x + 12.0f, link.y + 12.0f)) {
                link.x = k_new_x;
            }
            if (!map_is_solid(world_map, link.x + 4.0f, k_new_y + 12.0f) &&
                !map_is_solid(world_map, link.x + 12.0f, k_new_y + 16.0f)) {
                link.y = k_new_y;
            }

            link.knock_x *= 0.82f; // Amortecimento de inércia do recuo
            link.knock_y *= 0.82f;
        } else {
            link.knock_x = 0.0f;
            link.knock_y = 0.0f;
        }

        if (link.invuln_timer > 0) {
            link.invuln_timer--;
        }

        // --------------------------------------------------------------------
        // ATUALIZAÇÃO DO SUBSISTEMA DE ENTIDADES (IA, COMBATE E PROJÉTEIS)
        // --------------------------------------------------------------------
        entity_manager_update(world_map, link.x, link.y,
                              &link.hearts, &link.rupees,
                              &link.invuln_timer, &link.knock_x, &link.knock_y);

        // Atualizacao do Subsistema de Subarmas (Bumerangue, Vórtice do Pote Magico, Projeteis)
        subweapon_update(world_map, link.x, link.y, &link.rupees, &link.hearts);

        // Respawn de teste caso o Link zere os corações
        if (link.hearts <= 0) {
            link.hearts = 3;
            link.x = (world_map && world_map->is_authentic) ? 448.0f : 296.0f;
            link.y = (world_map && world_map->is_authentic) ? 624.0f : 176.0f;
            link.invuln_timer = 90;
            link.knock_x = 0.0f;
            link.knock_y = 0.0f;
            hal_audio_play_sound(SOUND_SECRET, 0.7f, 0.8f);
        }

        // Atualização da animação dos passos com frequência dinâmica proporcional
        if (link.is_moving && actual_speed > 0.0f) {
            link.anim_timer += (int)(actual_speed * 4.0f + 0.5f);
            if (link.anim_timer >= 32) {
                link.anim_frame = (link.anim_frame + 1) % 2;
                link.anim_timer = 0;
                float pitch = (link.anim_frame == 0) ? 0.94f : 1.06f;
                hal_audio_play_sound(SOUND_FOOTSTEP, 0.45f, pitch);
            }
        } else {
            link.anim_frame = 0;
            link.anim_timer = 0;
        }
        } // Fim do bloco de gameplay (se não estiver em diálogo ativo)

        // Atualização da Câmera Virtual Widescreen (Segue o Link com interpolação Lerp)
        camera_update(&camera, link.x, link.y, ctx->render_width, ctx->render_height, world_map);

        // --------------------------------------------------------------------
        // 3. RENDERIZAÇÃO NO FRAMEBUFFER VIRTUAL
        // --------------------------------------------------------------------
        // 1. Renderiza o mapa com Frustum Culling inteligente
        map_render(world_map, &camera);

        // 2. Renderiza as entidades ativas (Octoroks, Projéteis e Itens no chão)
        entity_manager_render(&camera);

        // 3. Desenha a entidade do Link nas coordenadas relativas da câmera
        draw_link(&link, &camera);

        // 4. Renderiza as subarmas e efeitos em voo (Bumerangue, Vórtice de ar)
        subweapon_render(&camera);

        // 3. Barra Superior de HUD (Status do Jogo fixo na tela)
        draw_rect(0, 0, ctx->render_width, 14, 0x0C1C0DFF);

        // 3 Corações de Vida de Zelda no canto superior esquerdo
        for (int h = 0; h < link.hearts; h++) {
            draw_heart(4 + (h * 9), 3);
        }

        // Indicador de Controle Conectado (Verde se gamepad 8BitDo ativo, cinza se teclado)
        const char* pad_name = hal_input_get_controller_name();
        u32 pad_indicator_color = pad_name ? 0x00FF66FF : 0x555555FF;
        draw_rect(36, 4, 10, 6, pad_indicator_color); // Ícone do controle
        hal_video_put_pixel(37, 3, pad_indicator_color);
        hal_video_put_pixel(44, 3, pad_indicator_color);

        // Mini Radar Analógico no HUD
        AnalogStick stick_hud = hal_input_get_left_stick();
        draw_rect(50, 3, 9, 8, 0x1A2E1CFF);
        hal_video_put_pixel(54, 7, 0x446644FF);
        if (stick_hud.magnitude > 0.05f) {
            int dot_x = 54 + (int)(stick_hud.x * 3.2f);
            int dot_y = 7  + (int)(stick_hud.y * 2.8f);
            u32 dot_color = (stick_hud.magnitude > 0.8f) ? 0xFFDD00FF : 0x00FFCCFF;
            hal_video_put_pixel(dot_x, dot_y, dot_color);
        }

        // Contador de Rupees (Gemas Verdes de Zelda)
        draw_rect(65, 4, 5, 6, 0x00FF88FF); // Gema verde
        hal_video_put_pixel(67, 3, 0x00FF88FF);
        hal_video_put_pixel(67, 10, 0x00FF88FF);

        // Indicador de Trilha Sonora BGM no HUD (Minish Woods: Turquesa, Hyrule: Dourado, Mudo: Cinza)
        BgmTrack current_bgm = hal_audio_get_current_bgm();
        u32 bgm_color = (current_bgm == BGM_MINISH_WOODS)     ? 0x00E5FFFF :
                        (current_bgm == BGM_HYRULE_OVERWORLD) ? 0xFFD700FF :
                                                                0x666666FF;
        draw_rect(76, 5, 3, 5, bgm_color);
        hal_video_put_pixel(79, 4, bgm_color);
        hal_video_put_pixel(80, 5, bgm_color);
        hal_video_put_pixel(80, 6, bgm_color);

        // Slot e Ícone da Subarma / Item Secundário Equipado [B]
        subweapon_render_hud_icon(88, 1);

        // Badge da Região Ativa no canto superior direito
        u32 reg_color = (s_current_region == REGION_USA) ? 0x4287F5FF :
                        (s_current_region == REGION_EUR) ? 0xF5A742FF : 0xF54242FF;
        draw_rect(ctx->render_width - 32, 2, 28, 10, reg_color);

        // 4. Balão de Diálogos e Retratos de Personagens (Ezlo / NPCs)
        dialogue_render();

        // --------------------------------------------------------------------
        // 5. APRESENTAÇÃO NA TELA (SDL2 GPU)
        // --------------------------------------------------------------------
        hal_video_render_frame();
    }

    if (s_sheet0)   texture_free(s_sheet0);
    if (s_sheet1)   texture_free(s_sheet1);
    if (s_link_tex) texture_free(s_link_tex);
    if (s_octo_tex) texture_free(s_octo_tex);

    entity_manager_shutdown();
    map_destroy(world_map);
    hal_audio_shutdown();
    hal_input_shutdown();
    hal_video_shutdown();
    printf("[SUCESSO] Aplicacao finalizada com exito!\n");
    return 0;
}
