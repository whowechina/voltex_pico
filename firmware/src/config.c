/*
 * Controller Config and Runtime Data
 * WHowe <github.com/whowechina>
 * 
 * Config is a global data structure that stores all the configuration
 * Runtime is something to share between files.
 */

#include "config.h"
#include "save.h"

voltex_cfg_t *voltex_cfg;

static voltex_cfg_t default_cfg = {
    .knob = {
        .units_per_turn = 80,
    },
    .light = {
        .level = 128,
    },
    .pedal = {
        .internal = true,
        .external = true,
    },
};

voltex_runtime_t voltex_runtime;

static void config_loaded()
{
    if (voltex_cfg->knob.units_per_turn == 0) {
        voltex_cfg->knob.units_per_turn = 80;
        config_changed();
    }
}

void config_changed()
{
    save_request(false);
}

void config_factory_reset()
{
    *voltex_cfg = default_cfg;
    save_request(true);
}

void config_init()
{
    voltex_cfg = (voltex_cfg_t *)save_alloc(sizeof(*voltex_cfg), &default_cfg, config_loaded);
}
