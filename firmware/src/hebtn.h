/*
 * Hall Effect Button Reader
 * WHowe <github.com/whowechina>
 */

#ifndef HEBTN_H
#define HEBTN_H

void hebtn_init();
void hebtn_update();

uint8_t hebtn_keynum();

bool hebtn_pressed(uint8_t chn);
bool hebtn_updated(uint8_t chn);
uint16_t hebtn_velocity(uint8_t chn);

uint8_t hebtn_analog(uint8_t chn);
uint16_t hebtn_raw(uint8_t chn);

void hebtn_calibrate_travel();

#endif
