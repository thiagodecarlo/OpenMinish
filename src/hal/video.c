#include "hal/video.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * src/hal/video.c - Implementação da Camada de Vídeo Virtual com SDL2
 * ============================================================================
 * Este arquivo implementa o renderizador que converte os pixels da nossa
 * tela virtual de GBA em uma janela moderna acelerada por hardware.
 */

// Variáveis de estado internas (estáticas para não vazar para outros arquivos)
static SDL_Window*   s_window   = NULL;
static SDL_Renderer* s_renderer = NULL;
static SDL_Texture*  s_texture  = NULL;
static HalVideoContext s_ctx;

bool hal_video_init(const char* window_title, int scale_factor, bool enable_widescreen) {
    // 1. Inicializa os subsistemas da biblioteca SDL2 (Video, Joystick, Gamepad)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
        printf("[ERRO HAL Video] Falha ao inicializar SDL2: %s\n", SDL_GetError());
        return false;
    }

    // 2. Configura as dimensões da tela virtual
    s_ctx.scale = (scale_factor < 1) ? 1 : scale_factor;
    s_ctx.widescreen = enable_widescreen;
    s_ctx.render_width  = enable_widescreen ? GBA_WIDE_WIDTH : GBA_SCREEN_WIDTH;
    s_ctx.render_height = GBA_SCREEN_HEIGHT;

    int window_width  = s_ctx.render_width * s_ctx.scale;
    int window_height = s_ctx.render_height * s_ctx.scale;

    // 3. Cria a janela gráfica no Sistema Operacional (Windows/Mac/Linux)
    s_window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!s_window) {
        printf("[ERRO HAL Video] Nao foi possivel criar a janela: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // 4. Cria o Renderizador com Aceleração de Hardware (GPU) e VSync
    s_renderer = SDL_CreateRenderer(
        s_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!s_renderer) {
        printf("[ERRO HAL Video] Falha ao criar renderer GPU: %s\n", SDL_GetError());
        SDL_DestroyWindow(s_window);
        SDL_Quit();
        return false;
    }

    // Garante que o aspecto não fique deformado caso o usuário redimensione a janela
    SDL_RenderSetLogicalSize(s_renderer, s_ctx.render_width, s_ctx.render_height);

    // 5. Aloca o Framebuffer Virtual na memória RAM
    // Cada pixel tem 4 bytes (RGBA): Vermelho, Verde, Azul e Alfa (transparência)
    size_t buffer_size = s_ctx.render_width * s_ctx.render_height * sizeof(u32);
    s_ctx.framebuffer = (u32*)malloc(buffer_size);

    if (!s_ctx.framebuffer) {
        printf("[ERRO HAL Video] Falha ao alocar memoria para o Framebuffer!\n");
        hal_video_shutdown();
        return false;
    }

    // Limpa a tela inicial com a cor preta
    memset(s_ctx.framebuffer, 0, buffer_size);

    // 6. Cria a Textura de Streaming na GPU (ponte entre a RAM e a Placa de Vídeo)
    // Usamos SDL_TEXTUREACCESS_STREAMING porque atualizamos os pixels todo frame
    s_texture = SDL_CreateTexture(
        s_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        s_ctx.render_width,
        s_ctx.render_height
    );

    if (!s_texture) {
        printf("[ERRO HAL Video] Falha ao criar textura de streaming: %s\n", SDL_GetError());
        hal_video_shutdown();
        return false;
    }

    // Configura filtro "Nearest Neighbor" para manter o visual retrô de pixel art nítido
    SDL_SetTextureScaleMode(s_texture, SDL_ScaleModeNearest);

    printf("[HAL Video] Inicializado com sucesso! Resolucao Virtual: %dx%d (Janela: %dx%d)\n",
           s_ctx.render_width, s_ctx.render_height, window_width, window_height);

    return true;
}

void hal_video_put_pixel(int x, int y, u32 color_rgba) {
    // Verificação de limites de tela (evita invasão de memória fora do array)
    if (x < 0 || x >= s_ctx.render_width || y < 0 || y >= s_ctx.render_height) {
        return;
    }
    // Fórmula clássica de 2D para índice em array unidimensional: y * largura + x
    s_ctx.framebuffer[y * s_ctx.render_width + x] = color_rgba;
}

u32 hal_video_get_pixel(int x, int y) {
    if (x < 0 || x >= s_ctx.render_width || y < 0 || y >= s_ctx.render_height || !s_ctx.framebuffer) {
        return 0;
    }
    return s_ctx.framebuffer[y * s_ctx.render_width + x];
}

void hal_video_clear(u32 color_rgba) {
    int total_pixels = s_ctx.render_width * s_ctx.render_height;
    for (int i = 0; i < total_pixels; i++) {
        s_ctx.framebuffer[i] = color_rgba;
    }
}

void hal_video_render_frame(void) {
    if (!s_renderer || !s_texture || !s_ctx.framebuffer) return;

    // 1. Envia os pixels da memória RAM para a textura na GPU
    SDL_UpdateTexture(
        s_texture,
        NULL,
        s_ctx.framebuffer,
        s_ctx.render_width * sizeof(u32) // Pitch: bytes por linha
    );

    // 2. Limpa o renderizador e desenha a textura esticada mantendo a proporção
    SDL_RenderClear(s_renderer);
    SDL_RenderCopy(s_renderer, s_texture, NULL, NULL);

    // 3. Apresenta o quadro pronto na tela (Swap Buffers sincronizado pelo VSync)
    SDL_RenderPresent(s_renderer);
}

void hal_video_shutdown(void) {
    if (s_ctx.framebuffer) {
        free(s_ctx.framebuffer);
        s_ctx.framebuffer = NULL;
    }
    if (s_texture) {
        SDL_DestroyTexture(s_texture);
        s_texture = NULL;
    }
    if (s_renderer) {
        SDL_DestroyRenderer(s_renderer);
        s_renderer = NULL;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = NULL;
    }
    printf("[HAL Video] Janela de video fechada e memoria liberada.\n");
}

const HalVideoContext* hal_video_get_context(void) {
    return &s_ctx;
}
