/*
 * WS2812B Lights Control (Base + Left and Right Gimbals)
 * WHowe <github.com/whowechina>
 */

#ifndef LIGHT_H
#define LIGHT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h"

void light_init();
void light_update();

uint32_t rgb32(uint32_t r, uint32_t g, uint32_t b, bool gamma_fix);
uint32_t rgb32_from_hsv(uint8_t h, uint8_t s, uint8_t v);

void light_set_button(uint8_t index, uint32_t color, bool hid);
void light_set_fx(uint8_t index, uint32_t color, bool hid);
void light_set_start(uint32_t color, bool hid);

void light_set_left_side(uint8_t index, uint32_t color, bool hid);
void light_set_right_side(uint8_t index, uint32_t color, bool hid);

void light_set_left_vol(uint8_t index, uint32_t color, bool hid);
void light_set_left_right(uint8_t index, uint32_t color, bool hid);

void light_set_main(uint8_t index, uint32_t color);
void light_set_left(uint8_t index, uint32_t color);
void light_set_right(uint8_t index, uint32_t color);

#endif
