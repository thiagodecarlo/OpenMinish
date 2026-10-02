#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <SDL.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    int count = SDL_NumJoysticks();
    printf("Total de Controles detectados pelo SDL2: %d\n\n", count);

    for (int i = 0; i < count; i++) {
        const char* name = SDL_JoystickNameForIndex(i);
        SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
        char guid_str[64];
        SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
        int is_gamecontroller = SDL_IsGameController(i);

        printf("Dispositivo #%d:\n", i);
        printf("  - Nome: %s\n", name ? name : "Desconhecido");
        printf("  - GUID: %s\n", guid_str);
        printf("  - Reconhecido como GameController: %s\n\n", is_gamecontroller ? "SIM" : "NAO (Apenas Joystick generico)");
    }

    SDL_Quit();
    return 0;
}
