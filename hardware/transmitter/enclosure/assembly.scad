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

// =============================================================================
// OpenDriveHub Transmitter Enclosure – Full Assembly
// =============================================================================
// Three zones arranged horizontally (front to back along Y axis):
//
//   Front (body)                                   Back
//   [Joystick Zone] → [Module Zone] → [Display Zone]
//        Y=0              Y=joy_d          Y=joy_d+mod_d
//
// Set 'explode' in parameters.scad to spread zones apart for visibility.

use <joystick/joystick_zone.scad>
use <modules/module_zone.scad>
use <display/display_zone.scad>

include <parameters.scad>

// Zone Y positions (front to back)
y_joystick = 0;
y_modules  = joy_zone_depth + explode;
y_display  = joy_zone_depth + module_zone_depth + 2 * explode;

module full_assembly() {
    // Zone 1: Joystick Zone (front)
    color(color_joystick)
        translate([0, y_joystick, 0])
            joystick_zone();

    // Zone 2: Module Zone (middle)
    color(color_modules)
        translate([0, y_modules, 0])
            module_zone();

    // Zone 3: Display Zone (rear)
    color(color_display)
        translate([0, y_display, 0])
            display_zone();
}

// Render
full_assembly();

// Dimension report
echo(str("=== OpenDriveHub Transmitter Enclosure ==="));
echo(str("  Zone width (uniform): ", zone_width, " mm"));
echo(str("  Joystick zone depth:  ", joy_zone_depth, " mm"));
echo(str("  Module zone depth:    ", module_zone_depth, " mm"));
echo(str("  Display zone depth:   ", display_zone_depth, " mm"));
echo(str("  Total depth:          ", total_depth, " mm"));
echo(str("  Zone height:          ", zone_height, " mm"));
echo(str("  Module grid:          ", bay_grid_cols, "x", bay_grid_rows,
         " = ", bay_grid_width, "x", bay_grid_depth, " mm"));
echo(str("  Print bed:            ", print_bed_x, "x", print_bed_y, " mm"));
echo(str("  Each zone fits bed:   ",
    zone_width <= print_bed_x && joy_zone_depth <= print_bed_y &&
    module_zone_depth <= print_bed_y && display_zone_depth <= print_bed_y
    ? "YES" : "NO"));
echo(str("  Explode distance:     ", explode, " mm"));
