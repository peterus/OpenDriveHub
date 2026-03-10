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
 * LVGL v8 configuration for OpenDriveHub transmitter display.
 *
 * Only the features actually used by the display driver are enabled to keep
 * binary size and RAM usage small on the ESP32.
 */

/* clang-format off */
#if 1   /* Set to 0 to disable this file */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* ── Color ──────────────────────────────────────────────────────────────── */

/** Color depth: 1 / 8 / 16 / 32 */
#define LV_COLOR_DEPTH 16

/** Swap the 2 bytes of 16-bit colors (set to 1 if pixels look wrong) */
#define LV_COLOR_16_SWAP 0

/* ── Memory ─────────────────────────────────────────────────────────────── */

/** Use the built-in heap allocator */
#define LV_MEM_CUSTOM 0

/** LVGL heap size in bytes.
 *  The sim build creates many more widget objects (scan cards, RSSI bars,
 *  trim panel, 2-column controls) so it needs a bigger heap. */
#ifdef NATIVE_SIM
#define LV_MEM_SIZE (128 * 1024U)
#else
#define LV_MEM_SIZE (32 * 1024U)
#endif

/* ── HAL ────────────────────────────────────────────────────────────────── */

/** Default display refresh period [ms] */
#define LV_DISP_DEF_REFR_PERIOD 30

/** Input device read period [ms] */
#define LV_INDEV_DEF_READ_PERIOD 30

/** Use a custom tick source (we call lv_tick_inc() manually) */
#define LV_TICK_CUSTOM 0

/** Pixels per inch – used for some internal size calculations */
#define LV_DPI_DEF 130

/* ── Drawing ────────────────────────────────────────────────────────────── */

#define LV_DRAW_COMPLEX 1

/* ── GPU ────────────────────────────────────────────────────────────────── */

#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_SWM341_DMAX 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0

/* ── Logging ────────────────────────────────────────────────────────────── */

#define LV_USE_LOG 0

/* ── Assertions ─────────────────────────────────────────────────────────── */

#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/* ── Debug ──────────────────────────────────────────────────────────────── */

#define LV_USE_PERF_MONITOR   0
#define LV_USE_MEM_MONITOR    0
#define LV_USE_REFR_DEBUG     0

/* ── Other ──────────────────────────────────────────────────────────────── */

#define LV_USE_USER_DATA 1
#define LV_ENABLE_GC 0

/* ── Compiler ───────────────────────────────────────────────────────────── */

#define LV_ATTRIBUTE_FAST_MEM
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 4

/* ── Fonts ──────────────────────────────────────────────────────────────── */

#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1   /* small labels */
#define LV_FONT_MONTSERRAT_14 1   /* body text    */
#define LV_FONT_MONTSERRAT_16 1   /* header       */
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* ── Themes ─────────────────────────────────────────────────────────────── */

#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC   1
#define LV_USE_THEME_MONO    0

/* ── Layouts ────────────────────────────────────────────────────────────── */

#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/* ── Widgets ────────────────────────────────────────────────────────────── */

#define LV_USE_ARC        0
#define LV_USE_BAR        1   /* channel value bars */
#define LV_USE_BTN        1   /* touch buttons      */
#define LV_USE_BTNMATRIX  1   /* required by BTN    */
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMG        0
#define LV_USE_LABEL      1   /* all text labels    */
#define LV_USE_LED        0
#define LV_USE_LINE       1   /* separator lines    */
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0

/* ── Extra components ───────────────────────────────────────────────────── */

#define LV_USE_ANIMIMG    0 /* needs LV_USE_IMG */
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      0
#define LV_USE_COLORWHEEL 0
#define LV_USE_FRAGMENT   0
#define LV_USE_IMGBTN     0 /* needs LV_USE_IMG */
#define LV_USE_IMGFONT    0
#define LV_USE_KEYBOARD   0 /* needs LV_USE_BTNMATRIX + LV_USE_TEXTAREA */
#define LV_USE_LED        0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      0 /* needs LV_USE_ARC */
#define LV_USE_MONKEY     0
#define LV_USE_MSGBOX     0 /* needs LV_USE_BTNMATRIX */
#define LV_USE_SNAPSHOT   0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0
#define LV_USE_MSG        0
#define LV_USE_IME_PINYIN 0

/* ── File system (all off) ──────────────────────────────────────────────── */

#define LV_USE_FS_STDIO  0
#define LV_USE_FS_POSIX  0
#define LV_USE_FS_WIN32  0
#define LV_USE_FS_FATFS  0

/* ── Decoder / encoder (all off) ─────────────────────────────────────────── */

#define LV_USE_PNG      0
#define LV_USE_BMP      0
#define LV_USE_SJPG     0
#define LV_USE_GIF      0
#define LV_USE_QRCODE   0
#define LV_USE_FREETYPE 0
#define LV_USE_RLOTTIE  0
#define LV_USE_FFMPEG   0

/* ── Demo / examples (off for firmware) ─────────────────────────────────── */

#define LV_BUILD_EXAMPLES           0
#define LV_USE_DEMO_WIDGETS         0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK       0
#define LV_USE_DEMO_STRESS          0
#define LV_USE_DEMO_MUSIC           0

#endif /* LV_CONF_H */
#endif /* "Content enable" */
/* clang-format on */
