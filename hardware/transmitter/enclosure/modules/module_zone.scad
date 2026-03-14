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
// OpenDriveHub Transmitter Enclosure – Module Zone
// =============================================================================
// Middle zone containing the 5×3 module grid (30×30mm bays).
// Modules are inserted from the top, secured with screws from behind.
// Electronics (ESP32 + TCA9548A mux) are mounted underneath the grid.
//
// Front edge has grooves to receive joystick zone tongues.
// Rear edge has tongues for connecting to the display zone.

use <../utils.scad>
include <../parameters.scad>

module module_zone() {
    w = zone_width;
    d = module_zone_depth;
    h = zone_height;

    // Grid position (centered on X, centered on Y)
    grid_w = bay_grid_cols * (bay_unit_size + bay_wall) + bay_wall;
    grid_d = bay_grid_rows * (bay_unit_size + bay_wall) + bay_wall;
    grid_x = (w - grid_w) / 2;
    grid_y = (d - grid_d) / 2;

    difference() {
        union() {
            // Zone shell
            zone_shell(w, d, h);

            // Module grid structure (raised from floor)
            translate([grid_x, grid_y, 0])
                _module_grid_structure(grid_w, grid_d);

            // Tongues on rear edge (connect to display zone)
            t_positions = tongue_positions(w);
            for (tx = t_positions) {
                translate([0, d, 0])
                    tongue(tx);
            }
        }

        // Grooves on front edge (receive joystick zone tongues)
        t_positions = tongue_positions(w);
        for (tx = t_positions) {
            groove(tx);
        }

        // Cable slots front and rear walls
        translate([w / 2, 0, h / 2])
            cable_slot();
        translate([w / 2, d, h / 2])
            cable_slot();

        // USB port opening (side wall, for ESP32 access)
        usb_x = w - wall_thickness - 0.1;
        translate([usb_x, d / 2 - esp32_usb_width / 2 - 2,
                   wall_thickness + 3])
            cube([wall_thickness + 0.2, esp32_usb_width + 4,
                  esp32_usb_height + 2]);
    }

    // ESP32 mount (underneath grid, on the floor)
    esp32_x = grid_x + grid_w / 2 - esp32_pcb_width / 2;
    esp32_y = grid_y + 5;
    translate([esp32_x, esp32_y, wall_thickness])
        _esp32_standoffs();

    // TCA9548A mount (next to ESP32)
    mux_x = grid_x + grid_w / 2 - mux_pcb_width / 2;
    mux_y = grid_y + grid_d - mux_pcb_length - 5;
    translate([mux_x, mux_y, wall_thickness])
        _mux_standoffs();
}

// Module grid: walls forming the 5×3 bay layout
module _module_grid_structure(grid_w, grid_d) {
    bay_pitch = bay_unit_size + bay_wall;
    grid_h = bay_depth + wall_thickness;

    // Grid sits at the top of the zone (bays open upward)
    z_offset = zone_height - grid_h;

    translate([0, 0, z_offset]) {
        difference() {
            // Solid grid block
            cube([grid_w, grid_d, grid_h]);

            // Bay openings (from top)
            for (col = [0:bay_grid_cols - 1]) {
                for (row = [0:bay_grid_rows - 1]) {
                    x = bay_wall + col * bay_pitch;
                    y = bay_wall + row * bay_pitch;
                    translate([x, y, wall_thickness])
                        cube([bay_unit_size, bay_unit_size, bay_depth + 0.1]);
                }
            }
        }

        // Screw bosses for module retention (rear face of each bay)
        for (col = [0:bay_grid_cols - 1]) {
            for (row = [0:bay_grid_rows - 1]) {
                x = bay_wall + col * bay_pitch + bay_unit_size / 2;
                y = bay_wall + (row + 1) * bay_pitch - bay_wall / 2;
                translate([x, y, wall_thickness + bay_depth / 2])
                    rotate([-90, 0, 0])
                        difference() {
                            cylinder(d = m3_screw_dia + 4, h = bay_wall);
                            translate([0, 0, -0.1])
                                cylinder(d = m3_screw_dia,
                                         h = bay_wall + 0.2);
                        }
            }
        }
    }
}

// Simple standoffs for ESP32
module _esp32_standoffs() {
    standoff_h = 3;
    for (x = [2, esp32_pcb_width - 2])
        for (y = [2, esp32_pcb_length - 2])
            translate([x, y, 0])
                cylinder(d = 4, h = standoff_h);
}

// Simple standoffs for TCA9548A mux
module _mux_standoffs() {
    standoff_h = 3;
    for (x = [2, mux_pcb_width - 2])
        for (y = [2, mux_pcb_length - 2])
            translate([x, y, 0])
                cylinder(d = 4, h = standoff_h);
}

// Preview
module_zone();
