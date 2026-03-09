/**
 * TFT_eSPI user setup for OpenDriveHub transmitter.
 *
 * Pin and dimension values must match odh::config::tx:: in Config.h.
 * TFT_eSPI requires preprocessor macros, so we duplicate the values here
 * rather than referencing constexpr constants.
 */

/* clang-format off */

/* ── Driver selection ───────────────────────────────────────────────────── */
#define ILI9341_DRIVER

/* ── Physical panel dimensions ──────────────────────────────────────────── */
#define TFT_WIDTH   240
#define TFT_HEIGHT  320

/* ── SPI pin mapping ────────────────────────────────────────────────────── */
#define TFT_MOSI  23
#define TFT_MISO  19
#define TFT_SCLK  18
#define TFT_CS     5
#define TFT_DC    27
#define TFT_RST   26
#define TFT_BL    32

/* ── Backlight polarity ─────────────────────────────────────────────────── */
#define TFT_BACKLIGHT_ON  1   /* HIGH = active-high */

/* ── Touch panel (XPT2046) ─────────────────────────────────────────────── */
#define TOUCH_CS   4    /* XPT2046 chip-select pin */

/* ── Fonts to compile in ────────────────────────────────────────────────── */
#define LOAD_GLCD    /* Font 1 – small, always useful                        */
#define LOAD_FONT2   /* Font 2 – slightly larger                             */

/* ── SPI bus speed ──────────────────────────────────────────────────────── */
#define SPI_FREQUENCY  40000000   /* 40 MHz */
