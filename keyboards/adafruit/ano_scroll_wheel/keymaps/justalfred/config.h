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


#pragma once

#define LAYER_STATE_8BIT

#ifdef TAPPING_TERM
    #undef TAPPING_TERM
#endif
#define TAPPING_TERM 500

#define MOUSEKEY_MOVE_DELTA 1

#ifdef RGBLIGHT_ENABLE
    #define RGBLIGHT_EFFECT_BREATHING
    #define RGBLIGHT_EFFECT_ALTERNATING
#endif

/* Joystick */
#define ANALOG_JOYSTICK_X_AXIS_PIN B6
#define ANALOG_JOYSTICK_Y_AXIS_PIN B5
#define POINTING_DEVICE_INVERT_X
#define POINTING_DEVICE_INVERT_Y
#define ANALOG_JOYSTICK_SPEED_REGULATOR 30
