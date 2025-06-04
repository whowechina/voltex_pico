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

static void disp_spin()
{
    printf("[Spin]\n");
    printf("  Units Per Turn: %d.\n", voltex_cfg->spin.units_per_turn);
    for (int i = 0; i < 2; i++) {
        printf("  Spinner %d: %s, %s.\n", i + 1,
               spin_present(i) ? "OK" : "ERROR",
               voltex_cfg->spin.reversed[i] ? "Reversed" : "Forward");
    }
}

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [light|spin]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    if (argc == 0) {
        disp_light();
        disp_spin();
        return;
    }

    const char *choices[] = {"light", "spin" };
    switch (cli_match_prefix(choices, count_of(choices), argv[0])) {
        case 0:
            disp_light();
            break;
        case 1:
            disp_spin();
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

static void handle_spin_rate(const char *rate)
{
    const char *usage = "Usage: spin rate <units_per_turn>\n"
                        "  units_per_turn: 20..255\n";
    int units = cli_extract_non_neg_int(rate, 0);
    if ((units < 20) || (units > 255)) {
        printf(usage);
        return;
    }

    voltex_cfg->spin.units_per_turn = units;
    config_changed();
    disp_spin();
}

static void handle_spin_invert(int id, const char *dir)
{
    const char *usage = "Usage: spin <id> <forward|reversed>\n"
                        "  id: 1..5\n";

    const char *choices[] = {"forward", "reversed"};
    int match = cli_match_prefix(choices, count_of(choices), dir);
    if (match < 0) {
        printf(usage);
        return;
    }

    voltex_cfg->spin.reversed[id] = match;

    config_changed();
    disp_spin();
}

static void handle_spin(int argc, char *argv[])
{
    const char *usage = "Usage: spin rate <units_per_turn>\n"
                        "       spin <id> <normal|reverse>\n"
                        "  units_per_turn: 20..255\n"
                        "  id: 1..5\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    const char *choices[] = { "1", "2", "rate" };
    int match = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (match < 0) {
        printf(usage);
        return;
    }

    if (match == 2) {
        handle_spin_rate(argv[1]);
        return;
    }

    handle_spin_invert(match, argv[1]);
}

static void handle_calibrate(int argc, char *argv[])
{
    const char *usage = "Usage: calibrate <origin|travel>\n";

    if (argc != 1) {
        printf(usage);
        return;
    }

    const char *choices[] = {"origin", "travel"};
    switch (cli_match_prefix(choices, count_of(choices), argv[0])) {
        case 0:
            hebtn_calibrate_origin();
            break;
        case 1:
            hebtn_calibrate_travel();
            break;
        default:
            printf(usage);
            break;
    }
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
    cli_register("spin", handle_spin, "Set spin rate.");
    cli_register("calibrate", handle_calibrate, "Calibrate the key sensors.");
    cli_register("debug", handle_debug, "Toggle debug features.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", handle_factory_reset, "Reset everything to default.");
}
