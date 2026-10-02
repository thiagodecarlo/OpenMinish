#ifndef GBA_TYPES_H
#define GBA_TYPES_H

/*
 * ============================================================================
 * include/gba/types.h - Definições de Tipos Primitivos de Largura Fixa
 * ============================================================================
 * Por que este arquivo é crucial para o nosso projeto acadêmico?
 * No C padrão, o tamanho de tipos como 'long' varia dependendo do Sistema
 * Operacional (Windows = 32 bits, Linux/Mac 64-bit = 64 bits).
 * Para garantir que nosso port rode IDENTICO no Windows, macOS e Linux,
 * usamos os tipos exatos de <stdint.h>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Tipos inteiros sem sinal (Unsigned - apenas números positivos)
typedef uint8_t   u8;   // 1 byte  (8 bits):  0 a 255
typedef uint16_t  u16;  // 2 bytes (16 bits): 0 a 65.535
typedef uint32_t  u32;  // 4 bytes (32 bits): 0 a 4.294.967.295
typedef uint64_t  u64;  // 8 bytes (64 bits)

// Tipos inteiros com sinal (Signed - números positivos e negativos)
typedef int8_t    s8;   // 1 byte  (8 bits):  -128 a +127
typedef int16_t   s16;  // 2 bytes (16 bits): -32.768 a +32.767
typedef int32_t   s32;  // 4 bytes (32 bits): -2.147.483.648 a +2.147.483.647
typedef int64_t   s64;  // 8 bytes (64 bits)

// Direções canônicas de movimentação e orientação
typedef enum {
    DIR_DOWN = 0,
    DIR_UP,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Tipos booleanos (Verdadeiro / Falso)
typedef uint8_t   bool8;
typedef uint32_t  bool32;

// Dimensões originais da tela física do Game Boy Advance
#define GBA_SCREEN_WIDTH   240
#define GBA_SCREEN_HEIGHT  160
#define GBA_TOTAL_PIXELS   (GBA_SCREEN_WIDTH * GBA_SCREEN_HEIGHT) // 38.400 pixels

// Dimensões para o nosso modo Widescreen 16:9 estendido (futura feature!)
#define GBA_WIDE_WIDTH     284 // Proporção 16:9 aproximada
#define GBA_WIDE_HEIGHT    160

#endif // GBA_TYPES_H
