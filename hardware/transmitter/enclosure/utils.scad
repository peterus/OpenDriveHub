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
// OpenDriveHub Transmitter Enclosure – Utility Modules
// =============================================================================

include <parameters.scad>

// --- Rounded Cube ------------------------------------------------------------
// A cube with rounded edges on the XY plane.
// size: [x, y, z], radius: corner radius
module rounded_cube(size, radius) {
    r = min(radius, min(size[0], size[1]) / 2);
    hull() {
        for (x = [r, size[0] - r])
            for (y = [r, size[1] - r])
                translate([x, y, 0])
                    cylinder(h = size[2], r = r);
    }
}

// --- Fully Rounded Cube (all edges) ------------------------------------------
module rounded_cube_3d(size, radius) {
    r = min(radius, min(size[0], min(size[1], size[2])) / 2);
    hull() {
        for (x = [r, size[0] - r])
            for (y = [r, size[1] - r])
                for (z = [r, size[2] - r])
                    translate([x, y, z])
                        sphere(r = r);
    }
}

// --- Screw Clearance Hole ----------------------------------------------------
// Vertical through-hole for a screw (clearance fit).
// dia: screw diameter, depth: hole depth, head_dia/head_h: counterbore
module screw_hole(dia, depth, head_dia = 0, head_h = 0) {
    cylinder(d = dia, h = depth);
    if (head_dia > 0 && head_h > 0) {
        translate([0, 0, depth - head_h])
            cylinder(d = head_dia, h = head_h + 0.1);
    }
}

// --- M3 Clearance Hole (convenience) -----------------------------------------
module m3_clearance_hole(depth) {
    screw_hole(
        dia     = m3_screw_dia,
        depth   = depth,
        head_dia = m3_screw_head_dia,
        head_h   = m3_screw_head_h
    );
}

// --- Heat-Set Insert Hole ----------------------------------------------------
// Hole designed for pressing in a brass heat-set insert from the top.
module heat_insert_hole(dia = m3_insert_hole_dia, depth = m3_insert_depth) {
    // Slightly tapered entry for easier insertion
    cylinder(d1 = dia + 0.4, d2 = dia, h = 0.8);
    cylinder(d = dia, h = depth);
}

// --- Mounting Post -----------------------------------------------------------
// Cylindrical post with a heat-set insert hole at the top.
module mounting_post(height, outer_dia = mount_post_dia,
                     insert_dia = m3_insert_hole_dia,
                     insert_depth = m3_insert_depth) {
    difference() {
        cylinder(d = outer_dia, h = height);
        translate([0, 0, height - insert_depth])
            heat_insert_hole(dia = insert_dia, depth = insert_depth + 0.1);
    }
}

// --- Mounting Post with Through-Hole -----------------------------------------
// Post with a clearance hole going all the way through (for bolting layers).
module mounting_post_through(height, outer_dia = mount_post_dia,
                             hole_dia = m3_screw_dia) {
    difference() {
        cylinder(d = outer_dia, h = height);
        translate([0, 0, -0.1])
            cylinder(d = hole_dia, h = height + 0.2);
    }
}

// --- Alignment Pin -----------------------------------------------------------
module alignment_pin(dia = alignment_pin_dia, height = alignment_pin_height) {
    // Chamfered tip for easier insertion
    cylinder(d = dia, h = height - 1);
    translate([0, 0, height - 1])
        cylinder(d1 = dia, d2 = dia - 1, h = 1);
}

// --- Alignment Hole ----------------------------------------------------------
module alignment_hole(dia = alignment_hole_dia, depth = alignment_hole_depth) {
    // Chamfered entry
    cylinder(d1 = dia + 0.5, d2 = dia, h = 0.5);
    cylinder(d = dia, h = depth);
}

// --- Cable Slot --------------------------------------------------------------
// Rectangular slot for routing cables between layers.
module cable_slot(width = cable_slot_width, height = cable_slot_height,
                  wall = wall_thickness) {
    translate([-width / 2, -wall / 2 - 0.1, 0])
        cube([width, wall + 0.2, height]);
}

// --- Rubber Foot Recess ------------------------------------------------------
module rubber_foot_recess(dia = 10, depth = 1.5) {
    cylinder(d = dia, h = depth);
}

// --- Plate with Mounting Holes -----------------------------------------------
// Generates mounting hole positions for a rectangular plate.
// Returns a list of [x, y] positions for use with for() loops.
// Usage: for (pos = plate_mount_positions(w, d)) translate(pos) ...
function plate_mount_positions(width, depth,
                               inset_x = mount_post_inset_x,
                               inset_y = mount_post_inset_y) =
    [
        [inset_x,         inset_y],          // Front-left
        [width - inset_x, inset_y],          // Front-right
        [inset_x,         depth - inset_y],  // Rear-left
        [width - inset_x, depth - inset_y],  // Rear-right
        [width / 2,       inset_y],          // Front-center
        [width / 2,       depth - inset_y],  // Rear-center
    ];

// --- Text Label (debossed) ---------------------------------------------------
module text_label(txt, size = 5, depth = 0.4) {
    linear_extrude(height = depth)
        text(txt, size = size, halign = "center", valign = "center",
             font = "Liberation Sans:style=Bold");
}

// --- Fillet (2D profile for hull operations) ---------------------------------
module fillet_2d(r) {
    offset(r = r) offset(delta = -r) children();
}

// --- Mirror Copy (creates original + mirrored copy) --------------------------
module mirror_copy(axis = [1, 0, 0]) {
    children();
    mirror(axis) children();
}

// --- Tongue (male half of tongue-and-groove joint) ---------------------------
// Placed on the rear edge of a zone (protrudes in +Y).
// x_pos: X position of tongue center on the zone edge.
module tongue(x_pos, z_pos = wall_thickness, w = tongue_width,
              h = tongue_height, d = tongue_depth) {
    translate([x_pos - w / 2, 0, z_pos])
        cube([w, d, h]);
}

// --- Groove (female half of tongue-and-groove joint) -------------------------
// Cut into the front edge of the next zone (receives tongue from -Y).
// x_pos: X position of groove center on the zone edge.
module groove(x_pos, z_pos = wall_thickness, w = tongue_width,
              h = tongue_height, d = tongue_depth,
              tol = tongue_tolerance) {
    translate([x_pos - w / 2 - tol, -0.1, z_pos - tol])
        cube([w + 2 * tol, d + tol + 0.1, h + 2 * tol]);
}

// --- Tongue positions for a given zone width ---------------------------------
// Evenly distributes tongue_count tongues across the width.
function tongue_positions(width, count = tongue_count) =
    [for (i = [0:count - 1]) width / (count + 1) * (i + 1)];

// --- Zone Shell (open-top box) -----------------------------------------------
// Creates a box shell for a zone: walls + floor, open on top.
module zone_shell(width, depth, height, radius = corner_radius) {
    difference() {
        rounded_cube([width, depth, height], radius);
        translate([wall_thickness, wall_thickness, wall_thickness])
            rounded_cube([width - 2 * wall_thickness,
                          depth - 2 * wall_thickness,
                          height + 0.1],
                         max(0.1, radius - wall_thickness));
    }
}

// --- Zone Screw Bosses -------------------------------------------------------
// Mounting bosses at corners and mid-points for screwing zones together or
// attaching top panels.
function zone_mount_positions(width, depth,
                              inset_x = mount_post_inset_x,
                              inset_y = mount_post_inset_y) =
    [
        [inset_x,         inset_y],
        [width - inset_x, inset_y],
        [inset_x,         depth - inset_y],
        [width - inset_x, depth - inset_y],
    ];
