#include "hal/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * ============================================================================
 * src/hal/input.c - Implementação da Camada de Entrada Digital + Analógica
 * ============================================================================
 */

static u16 s_keys_raw      = 0;
static u16 s_keys_current  = 0;
static u16 s_keys_previous = 0;
static u16 s_keys_pressed  = 0;
static u16 s_keys_released = 0;

static SDL_Gamepad* s_gamepad = NULL;
static char s_controller_name[128] = "Nenhum Gamepad Conectado";
static bool s_has_controller = false;

// Valores brutos dos eixos da alavanca analógica esquerda (-32768 a +32767)
static s16 s_raw_axis_x = 0;
static s16 s_raw_axis_y = 0;
static AnalogStick s_left_stick = { 0.0f, 0.0f, 0.0f };

static void open_controller(SDL_JoystickID instance_id) {
    if (s_gamepad) {
        SDL_CloseGamepad(s_gamepad);
        s_gamepad = NULL;
    }

    s_gamepad = SDL_OpenGamepad(instance_id);
    if (s_gamepad) {
        const char* name = SDL_GetGamepadName(s_gamepad);
        snprintf(s_controller_name, sizeof(s_controller_name), "%s", name ? name : "Gamepad Compativel");
        s_has_controller = true;
        printf("\n====================================================================\n");
        printf("[GAMEPAD CONECTADO] %s (Modo Gamepad SDL3 Ativo)\n", s_controller_name);
        printf("====================================================================\n\n");
    }
}

void hal_input_init(void) {
    s_keys_raw = 0;
    s_keys_current = 0;
    s_keys_previous = 0;
    s_keys_pressed = 0;
    s_keys_released = 0;
    s_raw_axis_x = 0;
    s_raw_axis_y = 0;
    s_left_stick.x = 0.0f;
    s_left_stick.y = 0.0f;
    s_left_stick.magnitude = 0.0f;

    s_has_controller = false;
    snprintf(s_controller_name, sizeof(s_controller_name), "Nenhum Gamepad Conectado");

    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0) {
        open_controller(gamepads[0]);
        SDL_free(gamepads);
    }

    if (!s_has_controller) {
        printf("[INPUT] Nenhum controle detectado na inicializacao. Usando Teclado.\n");
    }
}

static u16 map_key(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:      return KEY_UP;
        case SDLK_DOWN:
        case SDLK_S:      return KEY_DOWN;
        case SDLK_LEFT:
        case SDLK_A:      return KEY_LEFT;
        case SDLK_RIGHT:
        case SDLK_D:      return KEY_RIGHT;

        case SDLK_Z:
        case SDLK_J:
        case SDLK_SPACE:  return KEY_A;

        case SDLK_X:
        case SDLK_K:      return KEY_B;

        case SDLK_RETURN: return KEY_START;
        case SDLK_BACKSPACE:
        case SDLK_RSHIFT: return KEY_SELECT;

        case SDLK_Q:      return KEY_L;
        case SDLK_E:      return KEY_R;

        default:          return 0;
    }
}

static u16 map_controller_button(u8 button) {
    switch (button) {
        case SDL_GAMEPAD_BUTTON_DPAD_UP:        return KEY_UP;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return KEY_DOWN;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return KEY_LEFT;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return KEY_RIGHT;

        case SDL_GAMEPAD_BUTTON_SOUTH:          return KEY_A;
        case SDL_GAMEPAD_BUTTON_EAST:           return KEY_B;
        case SDL_GAMEPAD_BUTTON_WEST:           return KEY_A;
        case SDL_GAMEPAD_BUTTON_NORTH:          return KEY_B;

        case SDL_GAMEPAD_BUTTON_START:          return KEY_START;
        case SDL_GAMEPAD_BUTTON_BACK:           return KEY_SELECT;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return KEY_L;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return KEY_R;

        default:                                return 0;
    }
}

void hal_input_process_event(const SDL_Event* event) {
    if (!event) return;

    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
            if (!event->key.repeat) {
                s_keys_raw |= map_key(event->key.key);
            }
            break;

        case SDL_EVENT_KEY_UP:
            s_keys_raw &= ~map_key(event->key.key);
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            s_keys_raw |= map_controller_button(event->gbutton.button);
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            s_keys_raw &= ~map_controller_button(event->gbutton.button);
            break;

        // Captura do Movimento Analógico do 8BitDo / Xbox / PS5
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if (event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
                s_raw_axis_x = event->gaxis.value;
            } else if (event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
                s_raw_axis_y = event->gaxis.value;
            }
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
            if (!s_has_controller) {
                open_controller(event->gdevice.which);
            }
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            if (s_has_controller) {
                if (s_gamepad) SDL_CloseGamepad(s_gamepad);
                s_gamepad = NULL;
                s_has_controller = false;
                s_raw_axis_x = 0;
                s_raw_axis_y = 0;
                s_keys_raw = 0;
                snprintf(s_controller_name, sizeof(s_controller_name), "Nenhum Gamepad Conectado");
                printf("[INPUT] Gamepad desconectado.\n");
            }
            break;

        default:
            break;
    }
}

void hal_input_update(void) {
    // ------------------------------------------------------------------------
    // CÁLCULO DE DEADZONE CIRCULAR E VETOR NORMALIZADO 360°
    // ------------------------------------------------------------------------
    const float DEADZONE = 7000.0f; // Zona morta para anular drift do sensor
    const float MAX_VALUE = 30000.0f;

    float raw_x = (float)s_raw_axis_x;
    float raw_y = (float)s_raw_axis_y;

    // Distância euclidiana (magnitude do deslocamento da alavanca)
    float distance = sqrtf(raw_x * raw_x + raw_y * raw_y);

    if (distance < DEADZONE) {
        s_left_stick.x = 0.0f;
        s_left_stick.y = 0.0f;
        s_left_stick.magnitude = 0.0f;
    } else {
        // Normalização matemática progressiva entre 0.0 e 1.0
        float normalized_mag = (distance - DEADZONE) / (MAX_VALUE - DEADZONE);
        if (normalized_mag > 1.0f) normalized_mag = 1.0f;

        s_left_stick.magnitude = normalized_mag;
        s_left_stick.x = (raw_x / distance);
        s_left_stick.y = (raw_y / distance);
    }

    // Combina os botões físicos com emulação digital dos direcionais
    u16 keys = s_keys_raw;

    // Se o analógico estiver inclinado além de 40%, adiciona aos bits direcionais
    if (s_left_stick.magnitude > 0.40f) {
        if (s_left_stick.x > 0.38f)  keys |= KEY_RIGHT;
        if (s_left_stick.x < -0.38f) keys |= KEY_LEFT;
        if (s_left_stick.y > 0.38f)  keys |= KEY_DOWN;
        if (s_left_stick.y < -0.38f) keys |= KEY_UP;
    }

    s_keys_current  = keys;
    s_keys_pressed  = s_keys_current & ~s_keys_previous;
    s_keys_released = ~s_keys_current & s_keys_previous;
    s_keys_previous = s_keys_current;
}

AnalogStick hal_input_get_left_stick(void) {
    return s_left_stick;
}

bool hal_input_is_held(u16 key_mask) {
    return (s_keys_current & key_mask) != 0;
}

bool hal_input_is_pressed(u16 key_mask) {
    return (s_keys_pressed & key_mask) != 0;
}

bool hal_input_is_released(u16 key_mask) {
    return (s_keys_released & key_mask) != 0;
}

u16 hal_input_get_gba_keyinput(void) {
    return (~s_keys_current) & KEY_MASK_ALL;
}

const char* hal_input_get_controller_name(void) {
    return s_has_controller ? s_controller_name : NULL;
}

void hal_input_shutdown(void) {
    if (s_gamepad) {
        SDL_CloseGamepad(s_gamepad);
        s_gamepad = NULL;
    }
    s_has_controller = false;
    printf("[INPUT] Subsistema de entrada finalizado.\n");
}
