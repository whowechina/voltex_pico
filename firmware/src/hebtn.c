/*
 * Hall Effect Button Reader
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "hebtn.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "board_defs.h"

#include "config.h"

#define KEY_NUM ADC_KEY_NUM

#define PRESS_TRIGGER 250
#define RELEASE_TRIGGER 300

void hebtn_init()
{
    gpio_init(ADC_MUX_EN);
    gpio_init(ADC_MUX_A0);
    gpio_init(ADC_MUX_A1);
    gpio_init(ADC_MUX_A2);
    gpio_set_dir(ADC_MUX_EN, GPIO_OUT);
    gpio_set_dir(ADC_MUX_A0, GPIO_OUT);
    gpio_set_dir(ADC_MUX_A1, GPIO_OUT);
    gpio_set_dir(ADC_MUX_A2, GPIO_OUT);
    gpio_put(ADC_MUX_EN, 1);
    gpio_put(ADC_MUX_A0, 0);
    gpio_put(ADC_MUX_A1, 0);
    gpio_put(ADC_MUX_A2, 0);

    adc_init();
    adc_gpio_init(26 + ADC_CHANNEL);
    adc_select_input(ADC_CHANNEL);
    gpio_pull_down(26 + ADC_CHANNEL);

    // pwm mode for lower power ripple
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);
}

uint8_t hebtn_keynum()
{
    return KEY_NUM;
}

static uint16_t reading[KEY_NUM];
uint64_t reading_time;

static uint16_t dist[KEY_NUM];

uint16_t velocity[KEY_NUM];
bool updated[KEY_NUM];
bool pressed[KEY_NUM];

static inline void select_channel(int chn)
{
    static const uint8_t key_map[KEY_NUM] = ADC_KEY_CHN;

    uint8_t mask = key_map[chn];
    gpio_put(ADC_MUX_A0, mask & 1);
    gpio_put(ADC_MUX_A1, mask & 2);
    gpio_put(ADC_MUX_A2, mask & 4);
}

static inline uint16_t read_avg(int count)
{
    uint32_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += adc_read();
    }
    return sum / count;
}

static void read_sensor(int chn)
{
    reading[chn] = read_avg(2);
    if (voltex_runtime.debug.sensor) {
        if (chn == 0) {
            printf("\n");
        }
        printf(" %d:%4d,", chn, reading[chn]);
    }
    
}

void hebtn_update()
{
    reading_time = time_us_64();

    for (int i = 0; i < KEY_NUM; i++) {
        read_sensor(i);
        select_channel((i + 1) % KEY_NUM); // for the next reading
        sleep_us(2);
    }
}

uint16_t hebtn_velocity(uint8_t chn)
{
    return velocity[chn];
}

bool hebtn_pressed(uint8_t chn)
{
    return pressed[chn];
}

bool hebtn_updated(uint8_t chn)
{
    if (updated[chn]) {
        updated[chn] = false;
        return true;
    }
    return false;
}

uint8_t hebtn_analog(uint8_t chn)
{
    int analog = (dist[chn] - 100) * 255 / 400;
    if (analog < 0) {
        analog = 0;
    }
    if (analog > 255) {
        analog = 255;
    }

    return analog;
}

uint16_t hebtn_raw(uint8_t chn)
{
    return reading[chn];
}

static void read_sensors_avg(uint16_t avg[KEY_NUM])
{
    const int avg_count = 1000;
    uint32_t sum[KEY_NUM] = {0};

    for (int i = 0; i < avg_count; i++) {
        for (int j = 0; j < KEY_NUM; j++) {
            select_channel(j);
            sleep_us(2);
            read_sensor(j);
            sum[j] += reading[j];
        }
    }
    for (int i = 0; i < KEY_NUM; i++) {
        avg[i] = sum[i] / avg_count;
    }
}

void hebtn_calibrate_origin()
{
    printf("Keep all keys far far away from the sensors.\n");
    printf("Calibrating origin...\n");

    uint16_t origin[KEY_NUM];
    read_sensors_avg(origin);

    printf("Done.\n");
    for (int i = 0; i < KEY_NUM; i++) {
        voltex_cfg->sensor.origin[i] = origin[i];
        printf("Key %2d: %4d.\n", i, origin[i]);
    }

    config_changed();
}

void hebtn_calibrate_travel()
{
    printf("Calibrating key RELEASED...\n");

    uint16_t released[KEY_NUM] = {0};
    uint16_t pressed[KEY_NUM] = {0};

    read_sensors_avg(released);

    printf("Calibrating key PRESSED...\n");
    printf("Please press all keys down, not necessarily simultaneously.\n");

    uint16_t min[KEY_NUM] = {0};
    uint16_t max[KEY_NUM] = {0};
    for (int i = 0; i < KEY_NUM; i++) {
        min[i] = released[i];
        max[i] = released[i];
    }
    uint64_t stop = time_us_64() + 10000000;
    while (time_us_64() < stop) {
        hebtn_update();
        for (int i = 0; i < KEY_NUM; i++) {
            int val = hebtn_raw(i);
            if (val < min[i]) {
                min[i] = val;
            }
            if (val > max[i]) {
                max[i] = val;
            }
        }
    }

    bool success = true;
    for (int i = 0; i < KEY_NUM; i++) {
        int npole_val = max[i] - released[i];
        int spole_val = released[i] - min[i];
        bool npole = npole_val > 400;
        bool spole = spole_val > 400;
        if (npole != spole) {
            pressed[i] = npole ? max[i] - 50 : min[i] + 50;
            released[i] += npole ? 150 : -150;
        } else {
            printf("Key %d calibration failed. [%d-%d-%d].\n", i, min[i], released[i], max[i]);
            success = false;
            break;
        }
    }

    printf("Calibration %s.\n", success ? "succeeded" : "failed");

    if (!success) {
        return;
    }

    for (int i = 0; i < KEY_NUM; i++) {
        voltex_cfg->baseline[i].released = released[i];
        voltex_cfg->baseline[i].pressed = pressed[i];
        printf("Key %d: %4d -> %4d.\n", i + 1, released[i], pressed[i]);
    }

    config_changed();
}