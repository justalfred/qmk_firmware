/* Copyright 2022 Alfred Lee
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include QMK_KEYBOARD_H

typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP,
    TD_TRIPLE_TAP,
} td_state_t;

typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;

td_state_t cur_dance(qk_tap_dance_state_t *state);
void x_finished(qk_tap_dance_state_t *state, void *user_data);
void x_reset(qk_tap_dance_state_t *state, void *user_data);

enum {
    X_CTL,
    SOME_OTHER_DANCE
};

#define TD_X   TD(X_CTL)

enum sofle_layers {
    _NORTH,
    _EAST,
    _SOUTH,
    _WEST,
};

enum custom_keycodes {
    TAB_R = SAFE_RANGE,
    TAB_L
};

bool is_wheel_mode = false;


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 *   ,----------------.    spin dial to zoom
 *  /     | pgup |     \   tap center key to restore zoom to 100%
 * |------+------+------|  hold center key and turn wheel to alt-tab
 * | left | !!!! | right|  double tap to toggle wheel mode: cmd +/- or cmd mousewheel
 * |------+------+------|  triple tap to change orientation
 *  \     | pgdn |     /
 *   `----------------'--.
 *               |LED PWR|
 *               '-------'
 */
[_NORTH] = LAYOUT(
           RGB_HUI,
    TAB_L, TD_X,    TAB_R,
           RGB_HUD, RGB_TOG
),
[_EAST] = LAYOUT(
           TAB_L,
  RGB_HUD, TD_X,    RGB_HUI,
           TAB_R,   RGB_TOG
),
[_SOUTH] = LAYOUT(
           RGB_HUD,
    TAB_R, TD_X,    TAB_L,
           RGB_HUI, RGB_TOG
),
[_WEST] = LAYOUT(
           TAB_R,
  RGB_HUI, TD_X,    RGB_HUD,
           TAB_L,   RGB_TOG
)
};


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TD_X:
            break;
        case TAB_R:
            if (record->event.pressed) {
                tap_code16(A(G(KC_RGHT)));
            }
            return false;
        case TAB_L:
            if (record->event.pressed) {
                tap_code16(A(G(KC_LEFT)));
            }
            return false;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (get_mods() & MOD_BIT(KC_RCMD)) {
        clockwise ? tap_code(KC_TAB) : tap_code16(S(KC_TAB));
    } else if (is_wheel_mode) {
        register_code(KC_RCMD);
        SEND_STRING(SS_DELAY(50));
        clockwise ? tap_code(KC_WH_U) : tap_code(KC_WH_D);
        SEND_STRING(SS_DELAY(50));
        unregister_code(KC_RCMD);
    } else {
        clockwise ? tap_code16(G(KC_PPLS)) : tap_code16(G(KC_MINS));
    }
    return true;
}

td_state_t cur_dance(qk_tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->pressed) return TD_SINGLE_HOLD;
        else return TD_SINGLE_TAP;
    } else if (state->count == 2) {
        return TD_DOUBLE_TAP;
    } else if (state->count == 3) {
        return TD_TRIPLE_TAP;
    } else return TD_UNKNOWN;
}

// Create an instance of 'td_tap_t' for the 'x' tap dance.
static td_tap_t xtap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

void x_finished(qk_tap_dance_state_t *state, void *user_data) {
    xtap_state.state = cur_dance(state);
    switch (xtap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(G(KC_0));
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_RCMD);
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case TD_DOUBLE_TAP:
            is_wheel_mode = !is_wheel_mode;
            break;
        case TD_TRIPLE_TAP:
            set_single_persistent_default_layer((get_highest_layer(layer_state) + 1) % 4);
            break;
        default: break;
    }
}

void x_reset(qk_tap_dance_state_t *state, void *user_data) {
    switch (xtap_state.state) {
        case TD_SINGLE_HOLD:
            unregister_code(KC_RCMD);
            rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
        default: break;
    }
    xtap_state.state = TD_NONE;
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [X_CTL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, x_finished, x_reset)
};
