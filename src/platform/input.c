#include <stdio.h>
#ifdef ADS_LINUX
    #ifdef ADS_X11
        // #include "platform/linux/x_input.c"
        void _ads_input_nothing(void) {return;}
    #else
        #error "Only X11 support!"
    #endif
#else
    #error "Only linux support"
#endif

#include "platform/input.h"

static Input input;

void ads_input_debug_print_down(void) {
    bool first = true;
    for (uint32_t i=0; i<ADS_KEY_COUNT; i++) {
        if (input.key_down[i]) {
            if (first) {
                printf("Key down: ");
                first = false;
            }
            printf("%d ", i);
        }
    }
    if (!first) {
        printf("\n");
    }
}

bool ads_input_key_pressed(Key key) {
    return input.key_pressed[key];
}

bool ads_input_key_down(Key key) {
    return input.key_down[key];
}

bool ads_input_key_released(Key key) {
    return input.key_released[key];
}

#ifdef ADS_X11
#include <X11/Xutil.h>
void _handle_event(input_event_t ev) {
    if (ev.event_type == ADS_INPUT_KEY) {
        Key k = ADS_KEY_NULL;
        switch (ev.data.key.id) {
            case XK_a: { k = ADS_KEY_A; } break;
            case XK_b: { k = ADS_KEY_B; } break;
            case XK_c: { k = ADS_KEY_C; } break;
            case XK_d: { k = ADS_KEY_D; } break;
            case XK_e: { k = ADS_KEY_E; } break;
            case XK_f: { k = ADS_KEY_F; } break;
            case XK_g: { k = ADS_KEY_G; } break;
            case XK_h: { k = ADS_KEY_H; } break;
            case XK_i: { k = ADS_KEY_I; } break;
            case XK_j: { k = ADS_KEY_J; } break;
            case XK_k: { k = ADS_KEY_K; } break;
            case XK_l: { k = ADS_KEY_L; } break;
            case XK_m: { k = ADS_KEY_M; } break;
            case XK_n: { k = ADS_KEY_N; } break;
            case XK_o: { k = ADS_KEY_O; } break;
            case XK_p: { k = ADS_KEY_P; } break;
            case XK_q: { k = ADS_KEY_Q; } break;
            case XK_r: { k = ADS_KEY_R; } break;
            case XK_s: { k = ADS_KEY_S; } break;
            case XK_t: { k = ADS_KEY_T; } break;
            case XK_u: { k = ADS_KEY_U; } break;
            case XK_v: { k = ADS_KEY_V; } break;
            case XK_w: { k = ADS_KEY_W; } break;
            case XK_x: { k = ADS_KEY_X; } break;
            case XK_y: { k = ADS_KEY_Y; } break;
            case XK_z: { k = ADS_KEY_Z; } break;

            case XK_0: { k = ADS_KEY_0; } break;
            case XK_1: { k = ADS_KEY_1; } break;
            case XK_2: { k = ADS_KEY_2; } break;
            case XK_3: { k = ADS_KEY_3; } break;
            case XK_4: { k = ADS_KEY_4; } break;
            case XK_5: { k = ADS_KEY_5; } break;
            case XK_6: { k = ADS_KEY_6; } break;
            case XK_7: { k = ADS_KEY_7; } break;
            case XK_8: { k = ADS_KEY_8; } break;
            case XK_9: { k = ADS_KEY_9; } break;

            case XK_KP_0: { k = ADS_KEY_PAD_0; } break;
            case XK_KP_1: { k = ADS_KEY_PAD_1; } break;
            case XK_KP_2: { k = ADS_KEY_PAD_2; } break;
            case XK_KP_3: { k = ADS_KEY_PAD_3; } break;
            case XK_KP_4: { k = ADS_KEY_PAD_4; } break;
            case XK_KP_5: { k = ADS_KEY_PAD_5; } break;
            case XK_KP_6: { k = ADS_KEY_PAD_6; } break;
            case XK_KP_7: { k = ADS_KEY_PAD_7; } break;
            case XK_KP_8: { k = ADS_KEY_PAD_8; } break;
            case XK_KP_9: { k = ADS_KEY_PAD_9; } break;

            case XK_BackSpace: { k = ADS_KEY_BACKSPACE; }break;
            case XK_Tab:       { k = ADS_KEY_TAB;       }break;
            case XK_Return:    { k = ADS_KEY_RETURN;    }break;
            case XK_Escape:    { k = ADS_KEY_ESCAPE;    }break;
            case XK_Delete:    { k = ADS_KEY_DELETE;    }break;
            case XK_space:     { k = ADS_KEY_SPACE;     }break;
            case XK_Caps_Lock: { k = ADS_KEY_CAPSLOCK;  }break;

            case XK_Shift_L:   { k = ADS_KEY_SHIFT_LEFT;  }break;
            case XK_Shift_R:   { k = ADS_KEY_SHIFT_RIGHT; }break;
            case XK_Control_L: { k = ADS_KEY_CTRL_LEFT;   }break;
            case XK_Control_R: { k = ADS_KEY_CTRL_RIGHT;  }break;
            case XK_Alt_L:     { k = ADS_KEY_ALT_LEFT;    }break;
            case XK_Alt_R:     { k = ADS_KEY_ALT_RIGHT;   }break;
            case XK_Meta_L:    { k = ADS_KEY_META_LEFT;   }break;
            case XK_Meta_R:    { k = ADS_KEY_META_RIGHT;  }break;
            case XK_Super_L:   { k = ADS_KEY_SUPER_LEFT;  }break;
            case XK_Super_R:   { k = ADS_KEY_SUPER_RIGHT; }break;
            case XK_Hyper_L:   { k = ADS_KEY_HYPER_LEFT;  }break;
            case XK_Hyper_R:   { k = ADS_KEY_HYPER_RIGHT; }break;

            case XK_Left:  { k = ADS_KEY_LEFT ; } break;
            case XK_Up:    { k = ADS_KEY_UP   ; } break;
            case XK_Right: { k = ADS_KEY_RIGHT; } break;
            case XK_Down:  { k = ADS_KEY_DOWN ; } break;

            case XK_F1:  { k = ADS_KEY_F1;  } break;
            case XK_F2:  { k = ADS_KEY_F2;  } break;
            case XK_F3:  { k = ADS_KEY_F3;  } break;
            case XK_F4:  { k = ADS_KEY_F4;  } break;
            case XK_F5:  { k = ADS_KEY_F5;  } break;
            case XK_F6:  { k = ADS_KEY_F6;  } break;
            case XK_F7:  { k = ADS_KEY_F7;  } break;
            case XK_F8:  { k = ADS_KEY_F8;  } break;
            case XK_F9:  { k = ADS_KEY_F9;  } break;
            case XK_F10: { k = ADS_KEY_F10; } break;
            case XK_F11: { k = ADS_KEY_F11; } break;
            case XK_F12: { k = ADS_KEY_F12; } break;

            default: {
                printf("Not handled! \n");
            } break;
        }

        // printf("Input event %s key: %d. Got Key: %d \n", ev.data.key.pressed ? "pressed" : "released" , ev.data.key.id, k);

        if (ev.data.key.pressed) {
            input.key_pressed[k] = true;
            input.key_released[k] = false;
        }
        else if (ev.data.key.released) {
            input.key_released[k] = true;
            input.key_pressed[k] = false;
        }

        // printf("handle event: pressed: %d \t down: %d \t released: %d\n", input.key_pressed[k], input.key_down[k], input.key_released[k]);
    }
    else {
        fprintf(stderr, "NotImplemented!\n");
    }
}

void _handle_input_pre_events(void) {
    // printf("pre pre event: pressed: %d \t down: %d \t released: %d\n", input.key_pressed[ADS_KEY_A], input.key_down[ADS_KEY_A], input.key_released[ADS_KEY_A]);
    for (uint32_t i=0; i<ADS_KEY_COUNT; i++) {
        input.key_down[i] |= input.key_pressed[i];
        input.key_pressed[i] = false;
        input.key_released[i] = false;
    }
    // printf("post pre event: pressed: %d \t down: %d \t released: %d\n", input.key_pressed[ADS_KEY_A], input.key_down[ADS_KEY_A], input.key_released[ADS_KEY_A]);
}

void _handle_input_post_events(void) {
    // printf("pre post event: pressed: %d \t down: %d \t released: %d\n", input.key_pressed[ADS_KEY_A], input.key_down[ADS_KEY_A], input.key_released[ADS_KEY_A]);
    for (uint32_t i=0; i<ADS_KEY_COUNT; i++) {
        // Remove down and pressed if it's released
        input.key_down[i] &= !input.key_released[i];
        input.key_pressed[i] &= !input.key_released[i];

        // Remove pressed if already down
        input.key_pressed[i] &= !input.key_down[i];
    }
    // printf("post post event: pressed: %d \t down: %d \t released: %d\n", input.key_pressed[ADS_KEY_A], input.key_down[ADS_KEY_A], input.key_released[ADS_KEY_A]);

}
#endif
