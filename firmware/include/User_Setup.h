/*
 * Copyright (C) 2026 Peter Buchegger
 *
 * This file is part of OpenDriveHub.
 *
 * OpenDriveHub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDriveHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
