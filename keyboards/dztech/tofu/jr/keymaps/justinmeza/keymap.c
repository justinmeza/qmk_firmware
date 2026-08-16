/* Copyright 2022 DZTECH <moyi4681@live.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANT ABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <HTTP://WWW.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "unicode.h"

// #define DV1 0  // Programmer DVORAK
// #define DV2 1  // Shifted Programmer DVORAK
// #define QW1 0  // QWERTY
#define US1 0  // US ANSI QWERTY
#define TF1 1  // Team Fortress 2
#define FN1 2  // Functions
#define FN2 3  // More functions

/*
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (get_highest_layer(layer_state) > 0) {
        uint8_t layer = get_highest_layer(layer_state);

        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                uint8_t index = g_led_config.matrix_co[row][col];

                if (index >= led_min && index < led_max && index != NO_LED &&
                keymap_key_to_keycode(layer, (keypos_t){col,row}) > KC_TRNS) {
                    rgb_matrix_set_color(index, RGB_GREEN);
                }
            }
        }
    }
    return false;
}
*/

// Full-board flash in the new layer's color on every layer change, fading out.
#define LAYER_FLASH_MS 600

static uint32_t layer_flash_timer  = 0;
static uint8_t  layer_flash_layer  = 0;
static bool     layer_flash_active = false;

static const uint8_t layer_flash_colors[][3] = {
    [US1] = {RGB_BLUE},
    [TF1] = {RGB_ORANGE},
    [FN1] = {RGB_GREEN},
    [FN2] = {RGB_RED},
};

layer_state_t layer_state_set_user(layer_state_t state) {
    layer_flash_layer  = get_highest_layer(state | default_layer_state);
    layer_flash_timer  = timer_read32();
    layer_flash_active = true;
    return state;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state|default_layer_state);

    if (layer_flash_active) {
        uint32_t elapsed = timer_elapsed32(layer_flash_timer);
        if (elapsed >= LAYER_FLASH_MS) {
            layer_flash_active = false;
        } else {
            uint8_t fade = 255 - (uint8_t)((elapsed * 255) / LAYER_FLASH_MS);
            const uint8_t *c = layer_flash_colors[layer_flash_layer];
            for (uint8_t i = led_min; i < led_max; i++) {
                rgb_matrix_set_color(i, (c[0] * fade) / 255, (c[1] * fade) / 255, (c[2] * fade) / 255);
            }
            return false;
        }
    }

    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t index = g_led_config.matrix_co[row][col];

            if (index < led_min || index >= led_max || index == NO_LED) {
                continue;
            }

            uint16_t keycode = keymap_key_to_keycode(layer, (keypos_t){col,row});

            if (layer == FN2 && keycode == QK_BOOT) {
                // Blink the flash-mode key while right Fn is held so it's easy to find.
                if (timer_read32() % 500 < 250) {
                    rgb_matrix_set_color(index, RGB_RED);
                } else {
                    rgb_matrix_set_color(index, RGB_OFF);
                }
            } else if (keycode <= KC_TRNS) {
                rgb_matrix_set_color(index, RGB_OFF);
            }
        }
    }
    return false;
}

enum custom_keycodes {
    TF2_THANKS = SAFE_RANGE,
    TF2_MOVE_UP,
    TF2_GO_LEFT,
    TF2_GO_RIGHT,
    TF2_INCOMING,
    TF2_YES,
    TF2_NO,
    TF2_SPY,
    LAYOUT_CYC,  // tap: next layout; Shift+tap: previous layout
    IME_TOGG,    // toggle macOS IME: かな (Japanese) <-> 英数 (ABC)
};

static const uint8_t layout_order[] = { US1, TF1 };
static uint8_t layout_index = 0;
static bool ime_on = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case TF2_THANKS:
        if (record->event.pressed) {
            SEND_STRING("z2");
        }
        break;
    case TF2_MOVE_UP:
        if (record->event.pressed) {
            SEND_STRING("z4");
        }
        break;
    case TF2_GO_LEFT:
        if (record->event.pressed) {
            SEND_STRING("z5");
        }
        break;
    case TF2_GO_RIGHT:
        if (record->event.pressed) {
            SEND_STRING("z6");
        }
        break;
    case TF2_INCOMING:
        if (record->event.pressed) {
            SEND_STRING("x1");
        }
        break;
    case TF2_YES:
        if (record->event.pressed) {
            SEND_STRING("z7");
        }
        break;
    case TF2_NO:
        if (record->event.pressed) {
            SEND_STRING("z8");
        }
        break;
    case TF2_SPY:
        if (record->event.pressed) {
            SEND_STRING("x2");
        }
        break;
    case LAYOUT_CYC:
        if (record->event.pressed) {
            uint8_t n = ARRAY_SIZE(layout_order);
            if ((get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT) {
                layout_index = (layout_index + n - 1) % n;
            } else {
                layout_index = (layout_index + 1) % n;
            }
            layer_move(layout_order[layout_index]);
            return false;
        }
        break;
    case IME_TOGG:
        if (record->event.pressed) {
            ime_on = !ime_on;
            tap_code(ime_on ? KC_LNG1 : KC_LNG2);  // macOS: かな / 英数
            return false;
        }
        break;
    }
    return true;
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//    [DV1] = LAYOUT_65_ansi(
//        KC_DLR,               KC_AMPR, KC_LBRC, KC_LCBR, KC_RCBR, KC_LPRN, KC_EQL,  KC_ASTR, KC_RPRN, KC_PLUS, KC_RBRC, KC_EXLM, KC_HASH, KC_BSPC, KC_HOME,
//        KC_TAB,               KC_SCLN, KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    KC_SLSH, KC_AT,   KC_BSLS, KC_PGUP,
//        MT(MOD_LCTL, KC_ESC), KC_A,    KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_MINS,          KC_ENT,  KC_PGDN,
//        MO(DV2),              KC_QUOT, KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    MO(DV2),          KC_UP,   KC_END,
//        MO(FN1),              KC_LGUI, KC_LALT,                            KC_SPC,           TO(QW1), DF(QW1), MO(FN2),          KC_LEFT, KC_DOWN, KC_RIGHT
//    ),
//    [DV2] = LAYOUT_65_ansi(
//        KC_TILD,   KC_PERC, KC_7,    KC_5,    KC_3,    KC_1,    KC_9,    KC_0,    KC_2,    KC_4,    KC_6,    KC_8,    KC_GRV,  _______,   _______,
//        S(KC_TAB), KC_COLN, KC_LT,   KC_GT,   S(KC_P), S(KC_Y), S(KC_F), S(KC_G), S(KC_C), S(KC_R), S(KC_L), KC_QUES, KC_CIRC, KC_PIPE,   _______,
//        _______,   S(KC_A), S(KC_O), S(KC_E), S(KC_U), S(KC_I), S(KC_D), S(KC_H), S(KC_T), S(KC_N), S(KC_S), KC_UNDS,          S(KC_ENT), _______,
//        CW_TOGG,   KC_DQUO, S(KC_Q), S(KC_J), S(KC_K), S(KC_X), S(KC_B), S(KC_M), S(KC_W), S(KC_V), S(KC_Z), CW_TOGG,          _______,   _______,
//        _______,   _______, _______,                            _______,          _______, _______, _______,          _______, _______,   _______
//    ),
    // [QW1] = LAYOUT_65_ansi(
    //     KC_GRV,                KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_HOME,
    //     KC_TAB,                KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
    //     MT(MOD_LCTL, KC_ESC),  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGDN,
    //     KC_LSFT,               KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_END,
// //        MO(FN1),               KC_LGUI, KC_LALT,                            KC_SPC,           TO(TF1), DF(DV1), MO(FN2),          KC_LEFT, KC_DOWN, KC_RGHT
    //     MO(FN1),               KC_LGUI, KC_LALT,                            KC_SPC,           TO(TF1), _______, MO(FN2),          KC_LEFT, KC_DOWN, KC_RGHT
    // ),
    [US1] = LAYOUT_65_ansi(
        KC_GRV,                KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_HOME,
        KC_TAB,                KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP,
        MT(MOD_LCTL, KC_ESC),  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGDN,
        KC_LSFT,               KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_END,
        MO(FN1),               KC_LALT, KC_LGUI,                            KC_SPC,           LAYOUT_CYC, IME_TOGG, MO(FN2),       KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [TF1] = LAYOUT_65_ansi(
        _______, _______,    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,     _______,      _______,
        _______, TF2_YES,    KC_W,    KC_E,    _______, _______, _______, _______, _______, _______, _______, _______, _______,     _______,      _______,
        _______, KC_A,       KC_S,    KC_D,    KC_F,    _______, _______, _______, _______, _______, _______, _______,              _______,      _______,
        KC_LSFT, TF2_THANKS, TF2_NO,  TF2_SPY, _______, _______, _______, _______, _______, _______, _______, _______,              TF2_MOVE_UP,  _______,
//        _______, _______,    _______,                            _______,          TO(DV1), _______, _______,          TF2_GO_LEFT, TF2_INCOMING, TF2_GO_RIGHT
        _______, _______,    _______,                            _______,          _______, _______, _______,          TF2_GO_LEFT, TF2_INCOMING, TF2_GO_RIGHT
    ),
    [FN1] = LAYOUT_65_ansi(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,  _______,
        _______, RM_TOGG, RM_NEXT, RM_HUEU, RM_HUED, RM_SATU, RM_SATD, RM_VALU, RM_VALD, _______, KC_PSCR, KC_SCRL, KC_PAUS, _______, _______,
        KC_LCTL, RM_SPDU, RM_SPDD, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        KC_LSFT, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          KC_VOLU, KC_MUTE,
        _______, _______, _______,                            KC_MPLY,          _______, _______, _______,          KC_MPRV, KC_VOLD, KC_MNXT
    ),
    [FN2] = LAYOUT_65_ansi(
        EE_CLR,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, QK_BOOT, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          QK_RBT,  _______,
        _______, _______, _______, _______, _______, _______, NK_TOGG, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______,                            _______,          _______, _______, _______,          _______, _______, _______
    )
    /*
    [NAME] = LAYOUT_65_ansi(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______,                            _______,          _______, _______, _______,          _______, _______, _______
    ),
    */
};
