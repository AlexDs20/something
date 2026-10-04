#ifndef ADS_PLATFORM_INPUT_H
#define ADS_PLATFORM_INPUT_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ADS_KEY_NULL,

    ADS_KEY_BACKSPACE,
    ADS_KEY_TAB,
    ADS_KEY_RETURN,
    ADS_KEY_ESCAPE,
    ADS_KEY_DELETE,
    ADS_KEY_SPACE,
    ADS_KEY_CAPSLOCK,

    ADS_KEY_SHIFT_LEFT,
    ADS_KEY_SHIFT_RIGHT,
    ADS_KEY_CTRL_LEFT,
    ADS_KEY_CTRL_RIGHT,
    ADS_KEY_ALT_LEFT,
    ADS_KEY_ALT_RIGHT,
    ADS_KEY_META_LEFT,
    ADS_KEY_META_RIGHT,
    ADS_KEY_SUPER_LEFT,
    ADS_KEY_SUPER_RIGHT,
    ADS_KEY_HYPER_LEFT,
    ADS_KEY_HYPER_RIGHT,

    ADS_KEY_LEFT,
    ADS_KEY_UP,
    ADS_KEY_RIGHT,
    ADS_KEY_DOWN,

    ADS_KEY_0,
    ADS_KEY_1,
    ADS_KEY_2,
    ADS_KEY_3,
    ADS_KEY_4,
    ADS_KEY_5,
    ADS_KEY_6,
    ADS_KEY_7,
    ADS_KEY_8,
    ADS_KEY_9,

    ADS_KEY_PAD_0,
    ADS_KEY_PAD_1,
    ADS_KEY_PAD_2,
    ADS_KEY_PAD_3,
    ADS_KEY_PAD_4,
    ADS_KEY_PAD_5,
    ADS_KEY_PAD_6,
    ADS_KEY_PAD_7,
    ADS_KEY_PAD_8,
    ADS_KEY_PAD_9,

    ADS_KEY_F1,
    ADS_KEY_F2,
    ADS_KEY_F3,
    ADS_KEY_F4,
    ADS_KEY_F5,
    ADS_KEY_F6,
    ADS_KEY_F7,
    ADS_KEY_F8,
    ADS_KEY_F9,
    ADS_KEY_F10,
    ADS_KEY_F11,
    ADS_KEY_F12,

    ADS_KEY_A,
    ADS_KEY_B,
    ADS_KEY_C,
    ADS_KEY_D,
    ADS_KEY_E,
    ADS_KEY_F,
    ADS_KEY_G,
    ADS_KEY_H,
    ADS_KEY_I,
    ADS_KEY_J,
    ADS_KEY_K,
    ADS_KEY_L,
    ADS_KEY_M,
    ADS_KEY_N,
    ADS_KEY_O,
    ADS_KEY_P,
    ADS_KEY_Q,
    ADS_KEY_R,
    ADS_KEY_S,
    ADS_KEY_T,
    ADS_KEY_U,
    ADS_KEY_V,
    ADS_KEY_W,
    ADS_KEY_X,
    ADS_KEY_Y,
    ADS_KEY_Z,
    ADS_KEY_COUNT,
} Key;

typedef enum {
    ADS_INPUT_NONE,
    ADS_INPUT_KEY,

    // ADS_INPUT_TEXT,

    // ADS_INPUT_MOUSE_BUTTON,
    // ADS_INPUT_MOUSE_MOVE,
    // ADS_INPUT_MOUSE_WHEEL,

    // ADS_INPUT_GAMEPAD_BUTTON,
    // ADS_INPUT_GAMEPAD_AXIS,
    ADS_INPUT_EVENT_TYPE_COUNT,
} input_event_type_t;

typedef struct {
    input_event_type_t event_type;

    union {
        struct {
            uint32_t id;
            bool pressed;
            bool released;
        } key;

        // struct {
        //     uint32_t codepoint;
        // } text;

        // struct {
        //     uint32_t button;
        // } mouse_button;
    } data;
} input_event_t;

typedef struct {
    bool key_down[ADS_KEY_COUNT];
    bool key_pressed[ADS_KEY_COUNT];
    bool key_released[ADS_KEY_COUNT];
} Input;


#ifdef __cplusplus
extern "C" {
#endif

bool ads_input_key_pressed(Key key);
bool ads_input_key_down(Key key);
bool ads_input_key_released(Key key);

void ads_input_debug_print_down(void);
void _handle_event(input_event_t ev);
void _handle_input_pre_events(void);
void _handle_input_post_events(void);

#ifdef __cplusplus
}
#endif

#endif  // ADS_PLATFORM_INPUT_H
