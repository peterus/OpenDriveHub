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
// OpenDriveHub Transmitter Enclosure – Joystick Zone
// =============================================================================
// Front zone (closest to the user's body). Contains two D400-R4 joysticks
// with ergonomic grips on the left and right sides.
// Battery compartment is integrated underneath.
//
// Rear edge has tongues for connecting to the module zone.

use <../utils.scad>
include <../parameters.scad>

module joystick_zone() {
    w = zone_width;
    d = joy_zone_depth;
    h = zone_height;

    // Joystick positions (centered on X, centered on Y within this zone)
    joy_y = d / 2;
    joy_left_x  = w / 2 - joy_center_distance / 2;
    joy_right_x = w / 2 + joy_center_distance / 2;

    difference() {
        union() {
            // Zone shell (open-top box)
            zone_shell(w, d, h);

            // Tongues on rear edge (connect to module zone)
            t_positions = tongue_positions(w);
            for (tx = t_positions) {
                translate([0, d, 0])
                    tongue(tx);
            }

            // Grips (hanging down from sides)
            // Left grip
            translate([0, d * 0.15, 0])
                _grip_shape(false);
            // Right grip
            translate([w, d * 0.15, 0])
                _grip_shape(true);

            // Internal reinforcement around joystick holes
            for (jx = [joy_left_x, joy_right_x]) {
                translate([jx, joy_y, wall_thickness])
                    difference() {
                        cylinder(d = joy_panel_cutout + 8, h = 4);
                        translate([0, 0, -0.1])
                            cylinder(d = joy_panel_cutout, h = 4.2);
                    }
            }

            // Mounting posts at rear edge (heat-set inserts for module zone)
            mount_positions = zone_mount_positions(w, d);
            for (pos = mount_positions) {
                translate([pos[0], pos[1], 0])
                    mounting_post(h - 1);
            }
        }

        // Joystick panel cutouts (through the top wall)
        for (jx = [joy_left_x, joy_right_x]) {
            // Shaft hole through top
            translate([jx, joy_y, -0.1])
                cylinder(d = joy_panel_cutout, h = h + 0.2);

            // Mounting screw holes
            for (i = [0:3]) {
                angle = i * 90 + 45;
                translate([
                    jx + cos(angle) * joy_mounting_hole_pcd / 2,
                    joy_y + sin(angle) * joy_mounting_hole_pcd / 2,
                    -0.1
                ])
                    cylinder(d = joy_mounting_hole_dia, h = h + 0.2);
            }
        }

        // Battery compartment cavity (underneath, accessible from bottom/side)
        bat_cavity_w = battery_width + 2 * battery_clearance;
        bat_cavity_l = battery_length + 2 * battery_clearance;
        bat_cavity_h = battery_height + battery_clearance;
        translate([w / 2 - bat_cavity_w / 2,
                   d / 2 - bat_cavity_l / 2,
                   wall_thickness])
            cube([bat_cavity_w, bat_cavity_l, bat_cavity_h]);

        // Battery access opening (bottom)
        translate([w / 2 - battery_width / 2,
                   d / 2 - battery_length / 2,
                   -0.1])
            cube([battery_width, battery_length, wall_thickness + 0.2]);

        // Cable slot at rear wall (to module zone)
        translate([w / 2, d, h / 2])
            cable_slot();

        // Side cable slots
        for (x = [-0.1, w - wall_thickness]) {
            translate([x, d / 2 - cable_slot_width / 2, h / 2])
                cube([wall_thickness + 0.2, cable_slot_width, cable_slot_height]);
        }
    }
}

// Grip shape: extends downward from the zone side
module _grip_shape(is_right) {
    mirror_x = is_right ? 1 : 0;
    mirror([mirror_x, 0, 0])
    translate([0, 0, 0]) {
        hull() {
            // Top attachment (flush with zone side)
            translate([-wall_thickness, 0, -5])
                cube([wall_thickness, grip_depth, 10]);

            // Bottom grip section
            translate([-grip_width, grip_depth * 0.1, -grip_length + 10])
                rounded_cube_3d([grip_width, grip_depth * 0.8, 15],
                                grip_fillet);
        }
    }
}

// Joystick placeholder for visualization
module _joy_placeholder() {
    // Shaft above panel
    color([0.2, 0.2, 0.2])
        cylinder(d = 8, h = joy_shaft_height);
    color([0.1, 0.1, 0.1])
        translate([0, 0, joy_shaft_height - 15])
            sphere(d = 20);
    // Body below panel (inside the zone)
    color([0.5, 0.5, 0.5, 0.5])
        translate([0, 0, -joy_body_height])
            cylinder(d = joy_body_width * 0.8, h = joy_body_height);
}

// Battery cover: slides or clips into the bottom opening of the joystick zone.
// Print this part separately.
module battery_cover() {
    cover_tol  = 0.3;   // Fit tolerance on each side
    lip        = 1.5;   // Lip that sits inside the cavity to retain the cover
    cover_w    = battery_width - 2 * cover_tol;
    cover_l    = battery_length - 2 * cover_tol;
    cover_h    = wall_thickness;

    difference() {
        union() {
            // Main plate (flush with bottom of joystick zone)
            rounded_cube([cover_w, cover_l, cover_h], corner_radius);

            // Retention lip (sits inside the battery opening)
            translate([lip, lip, cover_h])
                cube([cover_w - 2 * lip, cover_l - 2 * lip, lip]);
        }

        // Finger-pull recess for easy removal
        translate([cover_w / 2, cover_l / 2, -0.1])
            cylinder(d = 15, h = cover_h * 0.6);
    }
}

// Preview
joystick_zone();

if (show_components) {
    w = zone_width;
    d = joy_zone_depth;
    joy_y = d / 2;
    joy_left_x  = w / 2 - joy_center_distance / 2;
    joy_right_x = w / 2 + joy_center_distance / 2;

    translate([joy_left_x, joy_y, zone_height])
        %_joy_placeholder();
    translate([joy_right_x, joy_y, zone_height])
        %_joy_placeholder();
}
