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
#include "hal/kinstone.h"
#include "hal/dungeon.h"
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
    int  max_hearts;
    int  rupees;
    int  invuln_timer;
    float knock_x;
    float knock_y;

    // Habilidade Lendária: Ataque Giratório (Tiger Scroll #1)
    bool has_spin_attack;
    bool is_charging_spin;
    int  spin_charge_timer;
    bool spin_ready;
    bool is_spinning;
    int  spin_timer;
    int  tiger_scroll_banner_timer;
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

static void spawn_overworld_entities(Tilemap* world_map) {
    entity_clear_all();
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
        // Mestre Espadachim Swiftblade no dojo/clareira de treinamento
        entity_spawn(ENTITY_NPC_SWIFTBLADE, 480.0f, 520.0f);
    } else {
        entity_spawn(ENTITY_ENEMY_OCTOROK, 160.0f, 220.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 420.0f, 150.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 340.0f, 310.0f);
        entity_spawn(ENTITY_ENEMY_KEESE, 200.0f, 130.0f);
        entity_spawn(ENTITY_ENEMY_CHUCHU, 320.0f, 220.0f);
        entity_spawn(ENTITY_NPC_FOREST_MINISH, 250.0f, 176.0f);
        entity_spawn(ENTITY_NPC_SWIFTBLADE, 260.0f, 220.0f);
    }
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

// Renderiza a moldura de um coração vazio no HUD (quando com dano)
static void draw_empty_heart(int hx, int hy) {
    u32 dark = 0x660808FF;
    u32 gray = 0x1E293BFF;

    hal_video_put_pixel(hx + 1, hy, dark);
    hal_video_put_pixel(hx + 2, hy, dark);
    hal_video_put_pixel(hx + 4, hy, dark);
    hal_video_put_pixel(hx + 5, hy, dark);

    for (int y = 1; y <= 3; y++) {
        for (int x = 0; x <= 6; x++) {
            bool border = (x == 0 || x == 6 || (y == 1 && x == 3));
            hal_video_put_pixel(hx + x, hy + y, border ? dark : gray);
        }
    }

    for (int x = 1; x <= 5; x++) hal_video_put_pixel(hx + x, hy + 4, (x == 1 || x == 5) ? dark : gray);
    for (int x = 2; x <= 4; x++) hal_video_put_pixel(hx + x, hy + 5, (x == 2 || x == 4) ? dark : gray);
    hal_video_put_pixel(hx + 3, hy + 6, dark);
}

// Renderiza efeitos de lâmina do Link: Golpe normal, Carga de Spin e Ataque Giratório 360°
static void draw_link_sword_effects(const Player* p, int px, int py) {
    u32 sword_steel = 0xCFE2F3FF;
    u32 sword_gold  = 0xFFD700FF;
    u32 cyan_glow   = 0x38BDF8FF;
    u32 cyan_white  = 0xBAE6FDFF;
    u32 white       = 0xFFFFFFFF;

    // 1. GOLPE DE ESPADA NORMAL (12 FRAMES)
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

    // 2. CARREGANDO O ATAQUE GIRATÓRIO (SPIN CHARGE)
    else if (p->is_charging_spin) {
        u32 blade_col = p->spin_ready ? cyan_white : sword_steel;
        u32 tip_spark = p->spin_ready ? 0x00FFFFFF : 0xFACC15FF;

        int tip_x = px + 8;
        int tip_y = py + 8;

        switch (p->dir) {
            case DIR_DOWN:
                draw_rect(px + 6, py + 15, 4, 9, blade_col);
                draw_rect(px + 4, py + 15, 8, 2, sword_gold);
                tip_x = px + 8; tip_y = py + 24;
                break;
            case DIR_UP:
                draw_rect(px + 6, py - 7, 4, 9, blade_col);
                draw_rect(px + 4, py + 0, 8, 2, sword_gold);
                tip_x = px + 8; tip_y = py - 7;
                break;
            case DIR_LEFT:
                draw_rect(px - 9, py + 9, 9, 4, blade_col);
                draw_rect(px + 0, py + 7, 2, 8, sword_gold);
                tip_x = px - 9; tip_y = py + 11;
                break;
            case DIR_RIGHT:
                draw_rect(px + 14, py + 9, 9, 4, blade_col);
                draw_rect(px + 13, py + 7, 2, 8, sword_gold);
                tip_x = px + 23; tip_y = py + 11;
                break;
        }

        // Faíscas cintilantes na ponta da lâmina acumulando energia
        int spark_ox = ((p->spin_charge_timer * 3) % 5) - 2;
        int spark_oy = ((p->spin_charge_timer * 5) % 5) - 2;
        hal_video_put_pixel(tip_x + spark_ox, tip_y + spark_oy, tip_spark);
        hal_video_put_pixel(tip_x - spark_ox, tip_y + spark_oy, white);

        // Anel de pulso celestial quando a carga máxima do Spin Attack está pronta!
        if (p->spin_ready) {
            int cx = px + 8;
            int cy = py + 8;
            int pulse_r = 13 + (p->anim_timer % 7);
            for (int a = 0; a < 8; a++) {
                float ang = (float)a * (3.14159265f / 4.0f) + (float)p->anim_timer * 0.12f;
                int ax = cx + (int)(cosf(ang) * (float)pulse_r);
                int ay = cy + (int)(sinf(ang) * (float)pulse_r);
                hal_video_put_pixel(ax, ay, cyan_glow);
            }
        }
    }

    // 3. EXECUÇÃO DO ATAQUE GIRATÓRIO (360° SPIN ATTACK)
    else if (p->is_spinning) {
        int cx = px + 8;
        int cy = py + 8;
        float progress = (float)(16 - p->spin_timer) / 16.0f; // 0.0 a 1.0

        // Lâmina giratória em alta velocidade
        float sword_ang = progress * 2.0f * 3.14159265f;
        int sw_x = cx + (int)(cosf(sword_ang) * 18.0f);
        int sw_y = cy + (int)(sinf(sword_ang) * 18.0f);
        draw_rect(sw_x - 1, sw_y - 1, 3, 3, cyan_white);

        // Rastro circular sweeping crescent arc (r = 21..25px)
        int num_trail_points = 36;
        for (int i = 0; i < num_trail_points; i++) {
            float ang = (float)i * (2.0f * 3.14159265f / (float)num_trail_points);
            int r_outer = 25;
            int r_inner = 21;

            int ox = cx + (int)(cosf(ang) * (float)r_outer);
            int oy = cy + (int)(sinf(ang) * (float)r_outer);
            int ix = cx + (int)(cosf(ang) * (float)r_inner);
            int iy = cy + (int)(sinf(ang) * (float)r_inner);

            u32 trail_col = ((i % 3) == 0) ? white : (((i % 2) == 0) ? cyan_white : cyan_glow);
            hal_video_put_pixel(ox, oy, trail_col);
            hal_video_put_pixel(ix, iy, cyan_glow);
        }

        // 4 faíscas dinâmicas ao redor da circunferência
        for (int s = 0; s < 4; s++) {
            float s_ang = (float)s * (3.14159265f / 2.0f) + sword_ang;
            int sx_spark = cx + (int)(cosf(s_ang) * 27.0f);
            int sy_spark = cy + (int)(sinf(s_ang) * 27.0f);
            draw_rect(sx_spark - 1, sy_spark - 1, 2, 2, white);
        }
    }
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

        int draw_y = py - 6;
        if (p->is_moving && p->anim_frame == 1) {
            draw_y -= 1; // Sutil bobbing de caminhada clássica
        }

        texture_draw_ex(s_link_tex, src_x, src_y, 16, 24, px, draw_y, flip_h);
        draw_link_sword_effects(p, px, py);
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

    // 6. Efeitos de espada e Spin Attack
    draw_link_sword_effects(p, px, py);
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("====================================================================\n");
    printf("   The Legend of Zelda: The Minish Cap - Native PC Port             \n");
    printf("   Camada de Input & Entidade Controlavel do Heroi (Link)           \n");
    printf("====================================================================\n");
    printf("Controles Disponiveis:\n");
    printf("  - Mover Link:     [WASD] ou [Setas do Teclado] ou [D-Pad/Analogico]\n");
    printf("  - Atacar / Acao:  [Z] ou [Espaco] ou [Botao A do Gamepad] (Atacar espada / Falar / Abrir bau!)\n");
    printf("  - Ataque Girat.:  Segurar [A] (Carregar espada -> Soltar para Spin Attack 360°!)\n");
    printf("  - Item Secundar.: [X] ou [Botao B do Gamepad] (Bumerangue / Pote Magico / Pegasus Boots!)\n");
    printf("  - Fusao Kinstone: [K] ou [Gatilho L no Gamepad] (Unir pedras da sorte com NPCs parceiros!)\n");
    printf("  - Ciclar Itens:   [Q] ou [Gatilho L no Gamepad] (Alternar item secundario equipado)\n");
    printf("  - Falar com Ezlo: [E] ou [Select no Gamepad] (Dicas e orientacoes do gorro companheiro!)\n");
    printf("  - Trilha Sonora:  [T] ou [Gatilho R no Gamepad] (Minish Woods / Hyrule / Deepwood / Mudo)\n");
    printf("  - Masmorra:       [D] Entrar/Sair de Deepwood Shrine (ou caminhar ao santuario ao norte!)\n");
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
    kinstone_init();
    dungeon_init();

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
    link.max_hearts = 3;
    link.rupees = 50;
    link.invuln_timer = 0;
    link.knock_x = 0.0f;
    link.knock_y = 0.0f;
    link.has_spin_attack = false;
    link.is_charging_spin = false;
    link.spin_charge_timer = 0;
    link.spin_ready = false;
    link.is_spinning = false;
    link.spin_timer = 0;
    link.tiger_scroll_banner_timer = 0;

    // Inicialização do Subsistema de Entidades e Spawn de Inimigos e NPCs
    entity_manager_init();
    spawn_overworld_entities(world_map);

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
                        case SDLK_k:
                            if (!kinstone_is_active() && !dialogue_is_active()) {
                                Entity* knpc = entity_find_kinstone_npc(link.x, link.y, 32.0f);
                                if (knpc) {
                                    kinstone_start_fusion(knpc);
                                }
                            }
                            break;
                        case SDLK_d:
                            if (dungeon_is_active()) {
                                dungeon_exit(&link.x, &link.y, &link.dir);
                            } else {
                                entity_clear_all();
                                dungeon_enter(&link.x, &link.y, &link.dir);
                            }
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

        if (kinstone_is_active()) {
            kinstone_update();

            bool confirm = hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_START);
            bool cancel  = hal_input_is_pressed(KEY_B);
            int  nav_dir = 0;

            if (hal_input_is_pressed(KEY_LEFT))  nav_dir = -1;
            if (hal_input_is_pressed(KEY_RIGHT)) nav_dir = 1;

            AnalogStick stick = hal_input_get_left_stick();
            static bool s_stick_was_left = false;
            static bool s_stick_was_right = false;
            if (stick.x < -0.5f && !s_stick_was_left) { nav_dir = -1; s_stick_was_left = true; }
            if (stick.x > -0.2f) s_stick_was_left = false;
            if (stick.x > 0.5f && !s_stick_was_right) { nav_dir = 1; s_stick_was_right = true; }
            if (stick.x < 0.2f) s_stick_was_right = false;

            kinstone_handle_input(confirm, cancel, nav_dir);
            link.is_moving = false;
        } else if (dialogue_is_active()) {
            dialogue_update();
            if (hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_START)) {
                dialogue_advance();
            }
            if (hal_input_is_held(KEY_B)) {
                dialogue_fast_forward();
            }
            link.is_moving = false;
        } else {
            // Recompensa do Mestre Swiftblade: Concede o Pergaminho do Tigre nº 1
            if (dialogue_is_swiftblade_reward_pending()) {
                dialogue_clear_swiftblade_reward();
                link.has_spin_attack = true;
                link.tiger_scroll_banner_timer = 200;
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.0f);
            }
            if (link.tiger_scroll_banner_timer > 0) {
                link.tiger_scroll_banner_timer--;
            }

            // Ação com Botão A: Primeiro interage com Masmorra / Baús Dourados / Swiftblade / NPCs, depois golpe de espada!
            if (hal_input_is_pressed(KEY_A) && !link.is_attacking && !link.is_spinning && !link.is_charging_spin) {
                if (dungeon_is_active()) {
                    if (dungeon_interact(link.x, link.y, &link.rupees, &link.hearts)) {
                        // Abriu o baú do altar da masmorra!
                    } else {
                        link.is_attacking = true;
                        link.attack_timer = 12;
                        hal_audio_play_sound(SOUND_SWORD_SLASH, 1.0f, 1.0f);
                    }
                } else if (entity_interact_chest(link.x, link.y, &link.rupees, &link.hearts)) {
                    // Abriu o baú dourado!
                } else {
                    Entity* nearby_swiftblade = entity_find_nearby_swiftblade(link.x, link.y, 28.0f);
                    if (nearby_swiftblade) {
                        dialogue_trigger_swiftblade_talk(link.has_spin_attack);
                    } else {
                        Entity* nearby_npc = entity_find_nearby_npc(link.x, link.y, 28.0f);
                        if (nearby_npc) {
                            dialogue_trigger_minish_talk();
                        } else {
                            link.is_attacking = true;
                            link.attack_timer = 12; // Dura 12 frames (0.2 segundos)
                            hal_audio_play_sound(SOUND_SWORD_SLASH, 1.0f, 1.0f);
                        }
                    }
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

                // Checa acerto contra inimigos (Octoroks, Keese, ChuChu) e projéteis
                entity_check_sword_hit(hit_x, hit_y, hit_w, hit_h, 1, link.dir);

                // Interação da espada com o cenário (cortar arbustos ou abrir baú no overworld)
                if (!dungeon_is_active() && map_interact_slash(world_map, hit_x + 6.0f, hit_y + 6.0f)) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.25f);
                    link.rupees += 5; // Recompensa clássica de Zelda!
                }

                link.attack_timer--;
                if (link.attack_timer <= 0) {
                    link.is_attacking = false;
                    // Se o herói possui o Pergaminho do Tigre e manteve o botão A segurado: inicia a carga!
                    if (link.has_spin_attack && hal_input_is_held(KEY_A)) {
                        link.is_charging_spin = true;
                        link.spin_charge_timer = 0;
                        link.spin_ready = false;
                    }
                }
            }

            // ----------------------------------------------------------------
            // MÁQUINA DE ESTADOS DO ATAQUE GIRATÓRIO (SPIN ATTACK FSM)
            // ----------------------------------------------------------------
            if (link.is_charging_spin) {
                if (hal_input_is_held(KEY_A)) {
                    link.spin_charge_timer++;
                    if (!link.spin_ready) {
                        if (link.spin_charge_timer % 20 == 0) {
                            hal_audio_play_sound(SOUND_SPIN_CHARGE, 0.70f, 1.0f);
                        }
                        if (link.spin_charge_timer >= 38) {
                            link.spin_ready = true;
                            hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.0f);
                        }
                    }
                } else {
                    // Botão A liberado pelo jogador!
                    if (link.spin_ready) {
                        // DISPARAR O ATAQUE GIRATÓRIO EM 360 GRAUS!
                        link.is_spinning = true;
                        link.spin_timer = 16;
                        hal_audio_play_sound(SOUND_SPIN_ATTACK, 1.0f, 1.0f);
                    }
                    link.is_charging_spin = false;
                    link.spin_ready = false;
                    link.spin_charge_timer = 0;
                }
            }

            // Execução da rotação e física de corte circular 360°
            if (link.is_spinning) {
                link.spin_timer--;
                int rot_step = (16 - link.spin_timer) / 4;
                Direction rot_dirs[4] = { DIR_DOWN, DIR_RIGHT, DIR_UP, DIR_LEFT };
                link.dir = rot_dirs[rot_step % 4];

                float spin_cx = link.x + 8.0f;
                float spin_cy = link.y + 8.0f;
                float spin_r = 26.0f;

                // 2 HP de dano duplicado e knockback radial centrífugo
                entity_check_spin_attack_hit(spin_cx, spin_cy, spin_r, 2);

                // Corte simultâneo de todos os arbustos no raio de 360 graus
                if (!dungeon_is_active()) {
                    int bushes = map_interact_spin(world_map, spin_cx, spin_cy, spin_r);
                    if (bushes > 0) {
                        hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.30f);
                        link.rupees += bushes * 5;
                    }
                }

                if (link.spin_timer <= 0) {
                    link.is_spinning = false;
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
                Entity* knpc = entity_find_kinstone_npc(link.x, link.y, 32.0f);
                if (knpc) {
                    kinstone_start_fusion(knpc);
                } else {
                    subweapon_cycle();
                }
            }
            if (hal_input_is_pressed(KEY_R)) {
                hal_audio_cycle_bgm();
            }

        // Modificador de Velocidade: Dash / Pegasus Boots (Botao B segurado) ou Carga da Espada
        float dash_mult = 1.0f;
        if (hal_input_is_held(KEY_B) && subweapon_get_current() == ITEM_PEGASUS_BOOTS) {
            dash_mult = 1.85f; // Arrancada veloz das Botas de Pegasus!
        } else if (link.is_charging_spin) {
            dash_mult = 0.85f; // Movimentação prudente enquanto acumula energia na lâmina
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
        } else if (!link.is_attacking && !link.is_spinning) {
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
        // SISTEMA DE COLISÃO COM O MAPA OU MASMORRA (DESLIZAMENTO SUAVE EM X E Y)
        // --------------------------------------------------------------------
        if (link.is_moving) {
            float new_x = link.x + move_x;
            float new_y = link.y + move_y;

            bool blocked_x = false;
            bool blocked_y = false;

            if (dungeon_is_active()) {
                blocked_x = dungeon_is_solid(new_x + 4.0f, link.y + 12.0f) ||
                            dungeon_is_solid(new_x + 12.0f, link.y + 12.0f) ||
                            dungeon_is_solid(new_x + 4.0f, link.y + 16.0f) ||
                            dungeon_is_solid(new_x + 12.0f, link.y + 16.0f);
                blocked_y = dungeon_is_solid(link.x + 4.0f, new_y + 12.0f) ||
                            dungeon_is_solid(link.x + 12.0f, new_y + 12.0f) ||
                            dungeon_is_solid(link.x + 4.0f, new_y + 16.0f) ||
                            dungeon_is_solid(link.x + 12.0f, new_y + 16.0f);
            } else {
                blocked_x = map_is_solid(world_map, new_x + 4.0f, link.y + 12.0f) ||
                            map_is_solid(world_map, new_x + 12.0f, link.y + 12.0f) ||
                            map_is_solid(world_map, new_x + 4.0f, link.y + 16.0f) ||
                            map_is_solid(world_map, new_x + 12.0f, link.y + 16.0f);
                blocked_y = map_is_solid(world_map, link.x + 4.0f, new_y + 12.0f) ||
                            map_is_solid(world_map, link.x + 12.0f, new_y + 12.0f) ||
                            map_is_solid(world_map, link.x + 4.0f, new_y + 16.0f) ||
                            map_is_solid(world_map, link.x + 12.0f, new_y + 16.0f);
            }

            if (!blocked_x) {
                link.x = new_x;
            }
            if (!blocked_y) {
                link.y = new_y;
            }

            if (dungeon_is_active()) {
                if (link.x < 0.0f) link.x = 0.0f;
                if (link.x > 256.0f - 16.0f) link.x = 256.0f - 16.0f;
                if (link.y < 0.0f) link.y = 0.0f;
                if (link.y > 160.0f - 16.0f) link.y = 160.0f - 16.0f;
            } else {
                // Confinamento dentro dos limites do mundo (640x480 pixels)
                if (link.x < 16.0f) link.x = 16.0f;
                if (link.x > (world_map->width * TILE_SIZE) - 32.0f) link.x = (world_map->width * TILE_SIZE) - 32.0f;
                if (link.y < 16.0f) link.y = 16.0f;
                if (link.y > (world_map->height * TILE_SIZE) - 32.0f) link.y = (world_map->height * TILE_SIZE) - 32.0f;

                // Checagem de Entrada no Deepwood Shrine pelo archway do santuário ao norte
                bool enter_shrine = false;
                if (world_map && world_map->is_authentic) {
                    if (link.x >= 436.0f && link.x <= 468.0f && link.y <= 485.0f && link.dir == DIR_UP) {
                        enter_shrine = true;
                    }
                } else {
                    if (link.x >= 280.0f && link.x <= 312.0f && link.y <= 40.0f && link.dir == DIR_UP) {
                        enter_shrine = true;
                    }
                }

                if (enter_shrine) {
                    entity_clear_all();
                    dungeon_enter(&link.x, &link.y, &link.dir);
                }
            }
        }

        // --------------------------------------------------------------------
        // FÍSICA DE KNOCKBACK DO HERÓI (RECÚO AO RECEBER DANO)
        // --------------------------------------------------------------------
        if (fabsf(link.knock_x) > 0.05f || fabsf(link.knock_y) > 0.05f) {
            float k_new_x = link.x + link.knock_x;
            float k_new_y = link.y + link.knock_y;

            bool k_blocked_x = false;
            bool k_blocked_y = false;

            if (dungeon_is_active()) {
                k_blocked_x = dungeon_is_solid(k_new_x + 4.0f, link.y + 12.0f) ||
                              dungeon_is_solid(k_new_x + 12.0f, link.y + 12.0f);
                k_blocked_y = dungeon_is_solid(link.x + 4.0f, k_new_y + 12.0f) ||
                              dungeon_is_solid(link.x + 12.0f, k_new_y + 16.0f);
            } else {
                k_blocked_x = map_is_solid(world_map, k_new_x + 4.0f, link.y + 12.0f) ||
                              map_is_solid(world_map, k_new_x + 12.0f, link.y + 12.0f);
                k_blocked_y = map_is_solid(world_map, link.x + 4.0f, k_new_y + 12.0f) ||
                              map_is_solid(world_map, link.x + 12.0f, k_new_y + 16.0f);
            }

            if (!k_blocked_x) link.x = k_new_x;
            if (!k_blocked_y) link.y = k_new_y;

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
        // ATUALIZAÇÃO DO SUBSISTEMA DE MASMORRA & ENTIDADES
        // --------------------------------------------------------------------
        static bool s_was_dungeon_active = false;
        if (s_was_dungeon_active && !dungeon_is_active()) {
            spawn_overworld_entities(world_map);
        }
        s_was_dungeon_active = dungeon_is_active();

        if (dungeon_is_active()) {
            dungeon_update(&link.x, &link.y, &link.dir, link.is_moving,
                           &link.hearts, &link.rupees);
        }

        entity_manager_update(world_map, link.x, link.y,
                              &link.hearts, &link.max_hearts, &link.rupees,
                              &link.invuln_timer, &link.knock_x, &link.knock_y);

        // Atualizacao do Subsistema de Subarmas (Bumerangue, Vórtice do Pote Magico, Projeteis)
        subweapon_update(world_map, link.x, link.y, &link.rupees, &link.hearts);

        // Respawn de teste caso o Link zere os corações
        if (link.hearts <= 0) {
            link.hearts = link.max_hearts;
            if (dungeon_is_active()) {
                link.x = 7.5f * TILE_SIZE;
                link.y = 8.0f * TILE_SIZE;
            } else {
                link.x = (world_map && world_map->is_authentic) ? 448.0f : 296.0f;
                link.y = (world_map && world_map->is_authentic) ? 624.0f : 176.0f;
            }
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

        // Atualização da Câmera Virtual Widescreen (Segue o Link ou centraliza na Masmorra)
        if (dungeon_is_active()) {
            camera.viewport_w = widescreen ? 284 : 240;
            camera.viewport_h = 160;
            camera.x = (float)(256 - camera.viewport_w) / 2.0f;
            camera.y = 0.0f;
        } else {
            camera_update(&camera, link.x, link.y, ctx->render_width, ctx->render_height, world_map);
        }

        // Aplicação do tremor de tela (Screen Shake) causado pelos passos gigantes e impacto do Chefe
        int shake_x = 0, shake_y = 0;
        entity_get_screen_shake(&shake_x, &shake_y);
        camera.x += (float)shake_x;
        camera.y += (float)shake_y;

        // --------------------------------------------------------------------
        // 3. RENDERIZAÇÃO NO FRAMEBUFFER VIRTUAL
        // --------------------------------------------------------------------
        if (dungeon_is_active()) {
            // Renderiza as câmaras subterrâneas, tochas, canais e portas da masmorra
            dungeon_render(&camera);
            // Renderiza inimigos ativos da câmara (Keese / ChuChu)
            entity_manager_render(&camera);
            // Desenha Link
            draw_link(&link, &camera);
            // Renderiza subarmas ativas (Bumerangue / Vórtice)
            subweapon_render(&camera);
            // Renderiza efeito suave de transição de salas (wipe)
            dungeon_render_transition(&camera);
        } else {
            // 1. Renderiza o mapa com Frustum Culling inteligente
            map_render(world_map, &camera);
            // 2. Renderiza as entidades ativas (Octoroks, Projéteis e Itens no chão)
            entity_manager_render(&camera);
            // 3. Desenha a entidade do Link nas coordenadas relativas da câmera
            draw_link(&link, &camera);
            // 4. Renderiza as subarmas e efeitos em voo (Bumerangue, Vórtice de ar)
            subweapon_render(&camera);
        }

        // 3. Barra Superior de HUD (Status do Jogo fixo na tela)
        draw_rect(0, 0, ctx->render_width, 14, 0x0C1C0DFF);

        // Corações de Vida de Zelda no canto superior esquerdo (Cheios e Vazios)
        for (int h = 0; h < link.max_hearts; h++) {
            if (h < link.hearts) {
                draw_heart(4 + (h * 9), 3);
            } else {
                draw_empty_heart(4 + (h * 9), 3);
            }
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

        // Indicador de Trilha Sonora BGM no HUD (Minish Woods: Turquesa, Hyrule: Dourado, Deepwood: Roxo, Boss: Vermelho, Mudo: Cinza)
        BgmTrack current_bgm = hal_audio_get_current_bgm();
        u32 bgm_color = (current_bgm == BGM_MINISH_WOODS)     ? 0x00E5FFFF :
                        (current_bgm == BGM_HYRULE_OVERWORLD) ? 0xFFD700FF :
                        (current_bgm == BGM_DEEPWOOD_SHRINE)  ? 0xA855F7FF :
                        (current_bgm == BGM_BOSS_BATTLE)      ? 0xEF4444FF :
                                                                0x666666FF;
        draw_rect(76, 5, 3, 5, bgm_color);
        hal_video_put_pixel(79, 4, bgm_color);
        hal_video_put_pixel(80, 5, bgm_color);
        hal_video_put_pixel(80, 6, bgm_color);

        // Slot e Ícone da Subarma / Item Secundário Equipado [B]
        subweapon_render_hud_icon(88, 1);

        // Contador de Chaves Pequenas da Masmorra (Small Keys 🔑 xN)
        dungeon_render_hud_keys(106, 2);

        // Ícone do Pergaminho do Tigre nº 1 no HUD (se Link dominou o Spin Attack)
        if (link.has_spin_attack) {
            int sx = 132;
            int sy = 2;
            draw_rect(sx, sy + 1, 9, 8, 0xFEF08AFF);
            draw_rect(sx - 1, sy, 11, 2, 0xD97706FF);
            draw_rect(sx - 1, sy + 8, 11, 2, 0xD97706FF);
            draw_rect(sx + 3, sy + 2, 3, 6, 0xDC2626FF); // Fita vermelha marcial
            hal_video_put_pixel(sx + 4, sy + 4, 0xFFFFFFFF);
        }

        // Badge da Região Ativa no canto superior direito
        u32 reg_color = (s_current_region == REGION_USA) ? 0x4287F5FF :
                        (s_current_region == REGION_EUR) ? 0xF5A742FF : 0xF54242FF;
        draw_rect(ctx->render_width - 32, 2, 28, 10, reg_color);

        // 4. Balão de Diálogos e Retratos de Personagens (Ezlo / NPCs)
        dialogue_render();

        // 5. Interface de Fusão de Kinstones (Pedras da Sorte)
        kinstone_render();

        // 6. Banner Festivo de Aquisição do Pergaminho do Tigre (Tiger Scroll #1)
        if (link.tiger_scroll_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0A0806EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xD4AF37FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x1A120EFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x2A1A12EE);

            // Ícone do Pergaminho dourado
            draw_rect(ban_x + 8, ban_y + 8, 10, 16, 0xFEF08AFF);
            draw_rect(ban_x + 7, ban_y + 6, 12, 3, 0xD97706FF);
            draw_rect(ban_x + 7, ban_y + 23, 12, 3, 0xD97706FF);
            draw_rect(ban_x + 8, ban_y + 14, 10, 4, 0xDC2626FF); // Fita escarlate

            font_draw_text(ban_x + 24, ban_y + 6, "PERGAMINHO DO TIGRE Nº 1!", 0xFDE047FF, true);
            font_draw_text(ban_x + 24, ban_y + 18, "ATAQUE GIRATORIO (SPIN ATTACK)!", 0x38BDF8FF, true);
        }

        // --------------------------------------------------------------------
        // 6. APRESENTAÇÃO NA TELA (SDL2 GPU)
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
