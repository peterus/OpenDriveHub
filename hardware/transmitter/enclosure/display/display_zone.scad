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
// OpenDriveHub Transmitter Enclosure – Display Zone
// =============================================================================
// Rear zone (farthest from the user's body). Contains the ILI9341 2.8" LCD
// mounted in landscape orientation with a bezel/viewing window.
//
// Front edge has grooves to receive module zone tongues.

use <../utils.scad>
include <../parameters.scad>

module display_zone() {
    w = zone_width;
    d = display_zone_depth;
    h = zone_height;

    // LCD position (centered on X and Y within this zone)
    lcd_x = (w - display_pcb_width) / 2;
    lcd_y = (d - display_pcb_depth) / 2;

    // LCD sits near the top surface
    lcd_z = h - display_module_thick - 2;

    bezel_overlap = 2.0; // Bezel overlaps the viewing area

    difference() {
        union() {
            // Zone shell
            zone_shell(w, d, h);

            // LCD support ledge (inside, around the LCD)
            ledge_h = 1.5;
            translate([lcd_x - 1, lcd_y - 1, lcd_z - ledge_h])
                difference() {
                    cube([display_pcb_width + 2, display_pcb_depth + 2,
                          ledge_h]);
                    translate([2, 2, -0.1])
                        cube([display_pcb_width - 2, display_pcb_depth - 2,
                              ledge_h + 0.2]);
                }

            // Mounting posts at front edge (through-holes for module zone screws)
            front_mounts = zone_mount_positions(w, d);
            for (pos = front_mounts) {
                translate([pos[0], pos[1], 0])
                    mounting_post_through(h - 1);
            }
        }

        // Grooves on front edge (receive module zone tongues)
        t_positions = tongue_positions(w);
        for (tx = t_positions) {
            groove(tx);
        }

        // LCD PCB pocket (slightly larger than PCB for insertion)
        translate([lcd_x - fit_tolerance, lcd_y - fit_tolerance, lcd_z])
            cube([display_pcb_width + 2 * fit_tolerance,
                  display_pcb_depth + 2 * fit_tolerance,
                  display_module_thick + 3]);

        // Display viewing window (through the top wall, reduced by bezel
        // overlap so the LCD is held in place by the frame)
        translate([lcd_x + display_view_offset_x + bezel_overlap,
                   lcd_y + display_view_offset_y + bezel_overlap,
                   -0.1])
            cube([display_view_width - 2 * bezel_overlap,
                  display_view_depth - 2 * bezel_overlap,
                  h + 0.2]);

        // LCD mounting screw holes
        hole_positions = [
            [lcd_x + display_mount_inset,
             lcd_y + display_mount_inset],
            [lcd_x + display_pcb_width - display_mount_inset,
             lcd_y + display_mount_inset],
            [lcd_x + display_mount_inset,
             lcd_y + display_pcb_depth - display_mount_inset],
            [lcd_x + display_pcb_width - display_mount_inset,
             lcd_y + display_pcb_depth - display_mount_inset],
        ];
        for (pos = hole_positions) {
            translate([pos[0], pos[1], lcd_z - 5])
                cylinder(d = display_mount_hole_dia, h = 10);
        }

        // Cable slot at front wall
        translate([w / 2, 0, h / 2])
            cable_slot();
    }
}

// LCD placeholder for visualization
module _lcd_placeholder() {
    // PCB
    color([0.1, 0.3, 0.6, 0.8])
        cube([display_pcb_width, display_pcb_depth, display_pcb_thick]);
    // Active display area
    color([0.05, 0.05, 0.05])
        translate([display_view_offset_x, display_view_offset_y,
                   display_pcb_thick])
            cube([display_view_width, display_view_depth, 1.5]);
}

// Preview
display_zone();

if (show_components) {
    w = zone_width;
    d = display_zone_depth;
    lcd_x = (w - display_pcb_width) / 2;
    lcd_y = (d - display_pcb_depth) / 2;
    lcd_z = zone_height - display_module_thick - 2;

    translate([lcd_x, lcd_y, lcd_z])
        %_lcd_placeholder();
}
