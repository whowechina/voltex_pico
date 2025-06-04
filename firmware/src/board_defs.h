/*
 * Voltex Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_voltex_PICO

#define RGB_PIN_MAIN 13
#define RGB_PIN_LEFT 14
#define RGB_PIN_RIGHT 15

#define RGB_ORDER GRB // or RGB

#define BUS_I2C i2c0
#define BUS_I2C_SDA 0
#define BUS_I2C_SCL 1
#define BUS_I2C_FREQ 400*1000

#define SPIN_DEF { 12, 11 }
 
 /* BT * 4, FX * 2, START, AUX * 2 */
#define BUTTON_DEF { 9, 8, 7, 6, 10, 5, 4, 3, 2 }
#define BUTTON_LIGHT_MAP { 1, 2, 4, 5, 0, 6, 3, 7, 8 }

#define ADC_MUX_EN 18
#define ADC_MUX_A0 21
#define ADC_MUX_A1 20
#define ADC_MUX_A2 19

#define ADC_CHANNEL 0

#define ADC_KEY_NUM 6
#define ADC_KEY_CHN { 1, 2, 3, 5, 0, 4 }

#else

#endif
