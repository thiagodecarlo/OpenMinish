cmake_minimum_required(VERSION 3.16)

# ==============================================================================
# PROJETO: OpenMinish - The Legend of Zelda: The Minish Cap Native PC Port
# Decompilação Autêntica C11 (zeldaret/tmc + Project Picori)
# ==============================================================================

project(OpenMinish VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# 1. Alvo Principal: Native OpenMinish PC Port via Xmake / build.py
# ------------------------------------------------------------------------------
# A compilação completa do port autêntico de The Minish Cap (621 arquivos C do jogo
# + backend Vulkan/OpenGL SDL3 + VirtuaAPU DirectSound) é gerenciada pelo pipeline:
#
#   python build.py --usa --slim
#   OU
#   xmake build tmc_pc
# ------------------------------------------------------------------------------

find_program(XMAKE_EXE xmake HINTS "C:/Program Files/xmake")
find_package(Python3 COMPONENTS Interpreter)

if(XMAKE_EXE)
    message(STATUS "Xmake detectado: ${XMAKE_EXE}")
    add_custom_target(tmc_pc ALL
        COMMAND ${XMAKE_EXE} build -y tmc_pc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compilando OpenMinish nativo (621 arquivos C autênticos) com Xmake..."
    )
    add_custom_target(openminish DEPENDS tmc_pc)
elseif(Python3_FOUND)
    message(STATUS "Python 3 detectado: ${Python3_EXECUTABLE}")
    add_custom_target(tmc_pc ALL
        COMMAND ${Python3_EXECUTABLE} build.py --usa --slim
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compilando OpenMinish via build.py..."
    )
    add_custom_target(openminish DEPENDS tmc_pc)
else()
    message(WARNING "Xmake ou Python 3 não encontrados no PATH. Instale o Xmake para compilar o projeto autêntico.")
endif()

# ------------------------------------------------------------------------------
# 2. Mock Engine Legado (Preservado para Fins Históricos em legacy/mock_engine)
# ------------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/legacy/mock_engine/src/main.c")
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.8/SDL3-3.2.8.zip
    )
    FetchContent_MakeAvailable(SDL3)

    file(GLOB LEGACY_HAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/legacy/mock_engine/src/hal/*.c")
    if(LEGACY_HAL_SOURCES)
        add_executable(legacy_mock_port EXCLUDE_FROM_ALL
            legacy/mock_engine/src/main.c
            ${LEGACY_HAL_SOURCES}
        )
        target_include_directories(legacy_mock_port PRIVATE
            include
            legacy/mock_engine/include
        )
        target_link_libraries(legacy_mock_port PRIVATE SDL3::SDL3)
    endif()
endif()
