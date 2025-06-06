/*
 * Controller Config
 * WHowe <github.com/whowechina>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t rgb_hsv; // 0: RGB, 1: HSV
    uint8_t val[3]; // RGB or HSV
} rgb_hsv_t;

typedef struct __attribute__((packed)) {
    struct {
        uint16_t up[6];
        uint16_t down[6];
    } calibrated;
    struct {
        uint8_t on[6];
        uint8_t off[6];
    } trigger;
    struct {
        uint8_t units_per_turn;
        uint8_t reversed[7];
    } knob;
    struct {
        rgb_hsv_t colors[12];
        uint8_t level;
        uint8_t reserved[15];
    } light;
} voltex_cfg_t;

typedef struct {
    struct {
        bool sensor;
        bool velocity;
    } debug;
} voltex_runtime_t;

extern voltex_cfg_t *voltex_cfg;
extern voltex_runtime_t voltex_runtime;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default

#endif
