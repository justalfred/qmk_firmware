/* Copyright 2023 Alfred Lee
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
    TD_DOUBLE_HOLD,
    TD_TRIPLE_TAP,
    TD_TRIPLE_HOLD
} td_state_t;

typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;

td_state_t cur_dance(qk_tap_dance_state_t *state);
void x_finished(qk_tap_dance_state_t *state, void *user_data);
void x_reset(qk_tap_dance_state_t *state, void *user_data);
void m_finished(qk_tap_dance_state_t *state, void *user_data);
void m_reset(qk_tap_dance_state_t *state, void *user_data);

enum {
    X_CTL,
    M_CTL
};

#define TD_X   TD(X_CTL)
#define TD_M   TD(M_CTL)

enum sofle_layers {
    _NORTH,
    _EAST,
    _SOUTH,
    _WEST,
};

bool is_wheel_mode = true;
bool is_on_autoscroll = false;
bool is_on_set_autoscroll = false;
bool is_tab_mode = false;
int8_t autoscroll_speed = 8;
bool autoscroll_neg = false;


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 *   ,----------------.
 *  /     | pgup |     \   tap center key to toggle autoscroll
 * |------+------+------|  hold center key and turn wheel to tune autoscroll
 * | left | !!!! | right|  double tap to toggle wheel mode: mousewheel or up/down
 * |------+------+------|  double tap, hold, and turn wheel for ctrl-tab
 *  \     | pgdn |     /   triple tap to change orientation
 *   `----------------'    triple tap, hold, and turn wheel for alt-tab
 */
[_NORTH] = LAYOUT_one_btn(
           KC_PGUP,
  KC_LEFT, TD_X,    KC_RGHT,
           KC_PGDN, TD_M
),
[_EAST] = LAYOUT_one_btn(
           KC_LEFT,
  KC_PGDN, TD_X,    KC_PGUP,
           KC_RGHT, TD_M
),
[_SOUTH] = LAYOUT_one_btn(
           KC_PGDN,
  KC_RGHT, TD_X,    KC_LEFT,
           KC_PGUP, TD_M
),
[_WEST] = LAYOUT_one_btn(
           KC_RGHT,
  KC_PGUP, TD_X,    KC_PGDN,
           KC_LEFT, TD_M
)
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TD_X:
            // rgblight_sethsv_noeeprom(HSV_CYAN);
            break;
        default:
            is_on_autoscroll = false;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (is_on_set_autoscroll) {
        if ((autoscroll_speed == 0) && (!clockwise != autoscroll_neg)) {  // can't decrease below 0
            autoscroll_neg = !autoscroll_neg;  // flip direction
        }
        if (clockwise != autoscroll_neg) {  // increase abs val
            autoscroll_speed += 2;
        } else {
            autoscroll_speed -= 2;
        }
        return true;
    }
    if (is_tab_mode) {
        clockwise ? tap_code(KC_TAB) : tap_code16(S(KC_TAB));
    } else if (is_wheel_mode) {
        clockwise ? tap_code(KC_WH_D) : tap_code(KC_WH_U);
    } else {
        clockwise ? tap_code(KC_DOWN) : tap_code(KC_UP);
    }
    is_on_autoscroll = false;
    return true;
}

uint32_t get_delay_ms(void) {
    if (is_wheel_mode) {
        return 1024 / autoscroll_speed;
    } else {
        return 8192 / autoscroll_speed;
    }
}

uint32_t autoscroll_callback(uint32_t trigger_time, void* cb_arg) {
    if (!is_on_autoscroll) {
        return 0;
    }
    if (is_wheel_mode) {
        autoscroll_neg ? tap_code(KC_WH_U) : tap_code(KC_WH_D);
    } else {
        autoscroll_neg ? tap_code(KC_UP) : tap_code(KC_DOWN);
    }
    return get_delay_ms();
}

td_state_t cur_dance(qk_tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->pressed) return TD_SINGLE_HOLD;
        else return TD_SINGLE_TAP;
    } else if (state->count == 2) {
        if (state->pressed) return TD_DOUBLE_HOLD;
        else return TD_DOUBLE_TAP;
    } else if (state->count == 3) {
        if (state->pressed) return TD_TRIPLE_HOLD;
        else return TD_TRIPLE_TAP;
    } else return TD_UNKNOWN;
}

// Create an instance of 'td_tap_t'
static td_tap_t tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

void x_finished(qk_tap_dance_state_t *state, void *user_data) {
    tap_state.state = cur_dance(state);
    static deferred_token autoscroll_token = INVALID_DEFERRED_TOKEN;
    switch (tap_state.state) {
        case TD_SINGLE_TAP:
            if (is_on_autoscroll) {
                cancel_deferred_exec(autoscroll_token);
            } else {
                autoscroll_token = defer_exec(get_delay_ms(), autoscroll_callback, NULL);
            }
            is_on_autoscroll = !is_on_autoscroll;
            break;
        case TD_SINGLE_HOLD:
            is_on_set_autoscroll = true;
            break;
        case TD_DOUBLE_TAP:
            is_wheel_mode = !is_wheel_mode;
            break;
        case TD_DOUBLE_HOLD:
            register_code(KC_LCTL);
            is_tab_mode = true;
            break;
        case TD_TRIPLE_TAP:
            layer_move((get_highest_layer(layer_state) + 1) % 4);
            break;
        case TD_TRIPLE_HOLD:
            register_code(host_keyboard_led_state().num_lock ? KC_LALT : KC_LGUI);
            is_tab_mode = true;
            break;
        default: break;
    }
}

void x_reset(qk_tap_dance_state_t *state, void *user_data) {
    switch (tap_state.state) {
        case TD_SINGLE_HOLD:
            is_on_set_autoscroll = false;
            break;
        case TD_DOUBLE_HOLD:
            unregister_code(KC_LCTL);
            is_tab_mode = false;
            break;
        case TD_TRIPLE_HOLD:
            unregister_code(host_keyboard_led_state().num_lock ? KC_LALT : KC_LGUI);
            is_tab_mode = false;
            break;
        default: break;
    }
    tap_state.state = TD_NONE;
}

void m_finished(qk_tap_dance_state_t *state, void *user_data) {
    tap_state.state = cur_dance(state);
    switch (tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_BTN1);
            break;
        case TD_SINGLE_HOLD:
            tap_code16(host_keyboard_led_state().num_lock ? A(KC_LEFT) : G(KC_LEFT));
            break;
        case TD_DOUBLE_TAP:
            tap_code(KC_ESC);
            break;
        case TD_DOUBLE_HOLD:
            tap_code16(host_keyboard_led_state().num_lock ? C(KC_W) : G(KC_W));
            break;
        default: break;
    }
}

void m_reset(qk_tap_dance_state_t *state, void *user_data) {
    tap_state.state = TD_NONE;
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [X_CTL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, x_finished, x_reset),
    [M_CTL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, m_finished, m_reset)
};
