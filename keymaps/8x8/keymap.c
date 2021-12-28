/* Copyright 2021 Jay Greco
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
#include "HT16K33_GFX.h"
#include <stdlib.h>

#define _BASE 0
#define _FUNC 1

bool numlock_set = false;

#define DISP_ADDR 0x70
HT16K33 *disp;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // Base layer (numpad)
  [_BASE] = LAYOUT(
           TO(_FUNC), KC_NLCK,   KC_KP_SLASH, 
  KC_KP_7, KC_KP_8,   KC_KP_9,   KC_KP_ASTERISK, 
  KC_KP_4, KC_KP_5,   KC_KP_6,   KC_KP_MINUS, 
  KC_KP_1, KC_KP_2,   KC_KP_3,   KC_KP_PLUS, 
  KC_NO,   KC_KP_0,   KC_KP_DOT, KC_KP_ENTER  
  ),

  // Function layer (numpad)
  [_FUNC] = LAYOUT(
             TO(_BASE), RGB_TOG, KC_NO,
    KC_NO,   KC_NO,     RGB_MOD, KC_NO,
    KC_NO,   KC_NO,     RGB_HUI, KC_NO,
    KC_NO,   KC_NO,     RGB_SAI, KC_NO,
    KC_NO,   KC_NO,     RGB_VAI, KC_NO
  ),
};

// Draw pixel while accounting for some weird wrap around shit
// Unsure if this is intended or if I just soldered something wrong
// disp->buf[0] = 0b10000000 is a pixel on the bottom left
// disp->buf[0] = 0b01000000 is a pixel on the upper left
// Note: only writes to the display buffer.
// Call HT16K33_refresh() to write to the display.
void drawPixel(HT16K33 *disp, uint8_t x, uint8_t y) {
    if ((x < 0) || (x > 7)) return;
    if ((y < 0) || (y > 7)) return;

    disp->buf[x] |= y == 0 ? 128 : (1 << (y-1));
}

void clearPixel(HT16K33 *disp, uint8_t x, uint8_t y) {
    if ((x < 0) || (x > 7)) return;
    if ((y < 0) || (y > 7)) return;

    disp->buf[x] &= y == 0 ? ~(128) : ~(1 << (y-1));
}

void drawHorizontal(HT16K33 *disp, uint8_t y, uint8_t start, uint8_t end) {
    for(; start <= end; start++) {
        drawPixel(disp, start, y);
    }
}
void drawFullHorizontal(HT16K33 *disp, uint8_t y) {
    drawHorizontal(disp, y, 0, 7);
}

void drawVertical(HT16K33 *disp, uint8_t x, uint8_t start, uint8_t end) {
    for(; start <= end; start++) {
        drawPixel(disp, x, start);
    }
}
void drawFullVertical(HT16K33 *disp, uint8_t x) {
    drawVertical(disp, x, 0, 7);
}

void matrix_init_kb(void) {
    matrix_init_user();
    disp = newHT16K33(8, DISP_ADDR);
    drawFullHorizontal(disp, 0);
    drawFullHorizontal(disp, 7);
    drawFullVertical(disp, 0);
    drawFullVertical(disp, 7);

    drawHorizontal(disp, 2, 2, 5);
    drawHorizontal(disp, 5, 2, 5);
    drawVertical(disp, 2, 2, 5);
    drawVertical(disp, 5, 2, 5);
    clearPixel(disp, 2, 2);

    HT16K33_refresh(disp);
}

void matrix_init_user(void) {
    matrix_init_remote_kb();
    register_code(KC_NLCK);
}

void matrix_scan_user(void) { matrix_scan_remote_kb(); }

void matrix_scan_kb(void) {
    matrix_scan_user();

}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_record_remote_kb(keycode, record);

    if (!numlock_set && record->event.pressed) {
        led_t led_state = host_keyboard_led_state();
        if (!led_state.num_lock) {
            register_code(KC_NLCK);
        }
        numlock_set = true;
    }

    return true;
}

void keyboard_post_init_user(void) {
  debug_enable=true;
//   debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return true;
}

void led_set_kb(uint8_t usb_led) {
    if (usb_led & (1 << USB_LED_NUM_LOCK))
        set_bitc_LED(LED_ON);
    else
        set_bitc_LED(LED_DIM);
}

