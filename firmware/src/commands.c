#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "save.h"
#include "cli.h"

#include "spin.h"
#include "hebtn.h"

#include "usb_descriptors.h"

#define SENSE_LIMIT_MAX 9
#define SENSE_LIMIT_MIN -9

static void disp_light()
{
    printf("[Light]\n");
    printf("  Level: %d.\n", voltex_cfg->light.level);
}

static void disp_knob()
{
    printf("[Knob]\n");
    printf("  Units Per Turn: %d.\n", voltex_cfg->knob.units_per_turn);
    for (int i = 0; i < 2; i++) {
        printf("  %s Knob: %s, %s.\n", i ? "Right" : "Left",
               spin_present(i) ? "OK" : "ERROR",
               voltex_cfg->knob.reversed[i] ? "Reverse" : "Forward");
    }
}

static void disp_hebtn()
{
    printf("[Hall Effect Button]\n");
    for (int i = 0; i < hebtn_keynum(); i++) {
        if (!hebtn_present(i)) {
            printf("  Key %d: Not Present.\n", i + 1);
            continue;
        }
        printf("  Key %d: %4d->%4d, On: %2d, Off: %2d.\n",
               i + 1, voltex_cfg->calibrated.up[i], voltex_cfg->calibrated.down[i],
                voltex_cfg->trigger.on[i] + 1, voltex_cfg->trigger.off[i] + 1);
    }
}

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [light|knob|he]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    if (argc == 0) {
        disp_light();
        disp_knob();
        disp_hebtn();
        return;
    }

    const char *choices[] = {"light", "knob", "he" };
    switch (cli_match_prefix(choices, count_of(choices), argv[0])) {
        case 0:
            disp_light();
            break;
        case 1:
            disp_knob();
            break;
        case 2:
            disp_hebtn();
            break;
        default:
            printf(usage);
            break;
    }
}

static void handle_level(int argc, char *argv[])
{
    const char *usage = "Usage: level <0..255>\n";
    if (argc != 1) {
        printf(usage);
        return;
    }

    int level = cli_extract_non_neg_int(argv[0], 0);
    if ((level < 0) || (level > 255)) {
        printf(usage);
        return;
    }

    voltex_cfg->light.level = level;
    config_changed();
    disp_light();
}

static void handle_knob_rate(const char *rate)
{
    const char *usage = "Usage: knob rate <units_per_turn>\n"
                        "  units_per_turn: 20..255\n";
    int units = cli_extract_non_neg_int(rate, 0);
    if ((units < 20) || (units > 255)) {
        printf(usage);
        return;
    }

    voltex_cfg->knob.units_per_turn = units;
    config_changed();
    disp_knob();
}

static void handle_knob_invert(int side, const char *dir)
{
    const char *usage = "Usage: knob <left|right> <forward|reversed>\n";

    const char *choices[] = {"forward", "reversed"};
    int match = cli_match_prefix(choices, count_of(choices), dir);

    if (match < 0) {
        printf(usage);
        return;
    }

    if ((side >= 0) && (side < 2)) {
        voltex_cfg->knob.reversed[side] = match;
    }

    config_changed();
    disp_knob();
}

static void handle_knob(int argc, char *argv[])
{
    const char *usage = "Usage: knob rate <units_per_turn>\n"
                        "       knob <left|right> <forward|reverse>\n"
                        "  units_per_turn: 20..255\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    const char *choices[] = { "left", "right", "rate" };
    int match = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (match < 0) {
        printf(usage);
        return;
    }

    if (match == 2) {
        handle_knob_rate(argv[1]);
        return;
    }

    handle_knob_invert(match, argv[1]);
}

static void handle_calibrate(int argc, char *argv[])
{
    hebtn_calibrate();
}

static void handle_trigger(int argc, char *argv[])
{
    const char *usage = "Usage: trigger <all|KEY> <ON> <OFF>\n"
                        "  KEY: 1..6\n"
                        "   ON: 1..36, distance for actuation.\n"
                        "  OFF: 1..36, distance for reset.\n";
    if (argc != 3) {
        printf(usage);
        return;
    }

    bool all_key = (strncasecmp(argv[0], "all", strlen(argv[0])) == 0);
    int key = cli_extract_non_neg_int(argv[0], 0) - 1;
    int on = cli_extract_non_neg_int(argv[1], 0) - 1;
    int off = cli_extract_non_neg_int(argv[2], 0) - 1;
    
    if ((!all_key && (key < 0)) || (key >= hebtn_keynum()) ||
        (on < 0) || (on > 35) || (off < 0) || (off > 35)) {
        printf(usage);
        return;
    }

    for (int i = 0; i < hebtn_keynum(); i++) {
        if (all_key || (i == key)) {
            voltex_cfg->trigger.on[i] = on;
            voltex_cfg->trigger.off[i] = off;
        }
    }
    config_changed();

    disp_hebtn();
}

static void handle_debug(int argc, char *argv[])
{
    const char *usage = "Usage: debug <sensor|velocity>\n";
    if (argc != 1) {
        printf(usage);
        return;
    }
    const char *choices[] = {"sensor", "velocity"};
    switch (cli_match_prefix(choices, count_of(choices), argv[0])) {
        case 0:
            voltex_runtime.debug.sensor ^= true;
            break;
        case 1:
            voltex_runtime.debug.velocity ^= true;
            break;
        default:
            printf(usage);
            break;
    }
}

static void handle_save()
{
    save_request(true);
}

static void handle_factory_reset()
{
    config_factory_reset();
    printf("Factory reset done.\n");
}

void commands_init()
{
    cli_register("display", handle_display, "Display all config.");
    cli_register("level", handle_level, "Set LED brightness level.");
    cli_register("knob", handle_knob, "Set knob rate and direction.");
    cli_register("calibrate", handle_calibrate, "Calibrate the key sensors.");
    cli_register("trigger", handle_trigger, "Set Hall effect switch triggering.");
    cli_register("debug", handle_debug, "Toggle debug features.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", handle_factory_reset, "Reset everything to default.");
}
