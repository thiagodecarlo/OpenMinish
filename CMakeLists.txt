cmake_minimum_required(VERSION 3.20)

# ==============================================================================
# PROJETO: OpenMinish - The Legend of Zelda: The Minish Cap Native PC Port
# Build System Universal: Windows (MSVC/GCC), macOS (Clang), Linux (GCC)
# ==============================================================================

project(OpenMinish C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Inclui o módulo FetchContent do CMake para baixar dependências automaticamente
include(FetchContent)

# ------------------------------------------------------------------------------
# 1. Configuração e Download Automático da SDL2 via GitHub Releases
# ------------------------------------------------------------------------------
message(STATUS "Configurando a biblioteca multiplataforma SDL2...")

set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-2.30.12.zip
)

FetchContent_MakeAvailable(SDL2)

# ------------------------------------------------------------------------------
# 2. Configuração do Nosso Executável Nativo (zelda_port)
# ------------------------------------------------------------------------------
add_executable(zelda_port
    src/main.c
    src/hal/video.c
    src/hal/texture.c
    src/hal/input.c
    src/hal/audio.c
    src/hal/map.c
    src/hal/entity.c
    src/hal/font.c
    src/hal/dialogue.c
    src/hal/subweapon.c
    src/hal/kinstone.c
    src/hal/dungeon.c
)

# Adiciona nossas pastas de cabeçalhos (.h) para que o C encontre #include "gba/types.h"
target_include_directories(zelda_port PRIVATE
    include
)

# Vincula as bibliotecas da SDL2 ao nosso executável
# SDL2main fornece o ponto de entrada WinMain no Windows de forma transparente
target_link_libraries(zelda_port PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
)

# No Windows, copia automaticamente a SDL2.dll para a mesma pasta do zelda_port.exe
if(WIN32)
    add_custom_command(TARGET zelda_port POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:zelda_port>
    )
endif()

# ------------------------------------------------------------------------------
# 3. Ferramenta de Extração de Assets Ahead-of-Time (asset_extractor)
# ------------------------------------------------------------------------------
# Este utilitário processa a ROM uma única vez para gerar os arquivos nativos,
# tornando a ROM totalmente desnecessária durante o gameplay.
add_executable(asset_extractor
    tools/extractor/main.c
    tools/extractor/lz77.c
    tools/extractor/gfx.c
)

target_include_directories(asset_extractor PRIVATE
    include
    tools/extractor
)
