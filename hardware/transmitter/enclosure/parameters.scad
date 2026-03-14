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
// OpenDriveHub Transmitter Enclosure – Central Parameters
// =============================================================================
// All dimensions in millimeters. Change values here to adjust the entire design.
//
// LAYOUT: Three zones arranged horizontally (front to back, Y axis):
//   [Joystick Zone] → [Module Zone] → [Display Zone]
// All zones share the same width (X). Each zone fits on the print bed.

// --- Print Bed Constraints ---------------------------------------------------
print_bed_x = 200;
print_bed_y = 200;

// --- General Construction ----------------------------------------------------
wall_thickness    = 2.5;   // Outer wall thickness
inner_wall        = 1.6;   // Internal divider walls
corner_radius     = 3.0;   // Outer corner rounding
fit_tolerance     = 0.3;   // General 3D-print fit tolerance

// --- Screw & Fastener Dimensions ---------------------------------------------
// M3 Heat-Set Insert (standard M3×5×5 brass insert)
m3_screw_dia      = 3.2;   // M3 clearance hole
m3_screw_head_dia = 5.8;   // M3 socket head cap screw head
m3_screw_head_h   = 3.0;   // M3 head height
m3_insert_dia     = 4.6;   // Heat-set insert outer diameter
m3_insert_depth   = 5.0;   // Heat-set insert depth
m3_insert_hole_dia = 4.8;  // Hole diameter for heat-set insert (slightly larger)

// --- Zone Connection (tongue-and-groove between zones) -----------------------
tongue_width      = 15.0;  // Width of each tongue
tongue_height     = 8.0;   // Height of tongue (Z)
tongue_depth      = 5.0;   // How far the tongue protrudes (Y)
tongue_tolerance  = 0.3;   // Gap for fit
tongue_count      = 3;     // Number of tongues per joint edge

// --- Joystick D400-R4 --------------------------------------------------------
// JH-D400X-R4 3-axis joystick (panel-mount)
joy_body_width    = 49.6;  // Joystick body width (X)
joy_body_length   = 94.5;  // Joystick body length (Y)
joy_body_height   = 35.0;  // Body height below panel
joy_shaft_height  = 40.0;  // Shaft height above panel
joy_panel_cutout  = 28.0;  // Circular panel cutout diameter
joy_mounting_dia  = 36.0;  // Mounting ring outer diameter
joy_mounting_hole_dia = 3.2; // Mounting screw holes
joy_mounting_hole_pcd = 40.0; // Mounting hole pitch circle diameter

// Ergonomic spacing
joy_center_distance   = 125; // Center-to-center distance between joysticks

// --- Module Bay --------------------------------------------------------------
bay_unit_size     = 30.0;  // Single bay unit (30×30mm)
bay_depth         = 25.0;  // Bay depth (how deep the module sits)
bay_wall          = 1.2;   // Wall between adjacent bays
bay_screw_inset   = 4.0;   // Screw hole distance from bay edge (rear mounting)

// Grid configuration
bay_grid_cols     = 5;     // Number of columns (along X)
bay_grid_rows     = 3;     // Number of rows (along Y)

// Computed grid dimensions
bay_grid_width  = bay_grid_cols * bay_unit_size + (bay_grid_cols + 1) * bay_wall;
bay_grid_height = bay_grid_rows * bay_unit_size + (bay_grid_rows + 1) * bay_wall;

// --- Display (ILI9341 2.8" TFT LCD, mounted LANDSCAPE) ----------------------
// In landscape orientation: long edge = X (width), short edge = Y (depth)
display_pcb_width    = 86.0;   // PCB along X (landscape)
display_pcb_depth    = 50.0;   // PCB along Y (landscape)
display_pcb_thick    = 1.6;    // PCB thickness
display_module_thick = 5.0;    // Total module thickness (PCB + components)

display_view_width   = 57.6;   // Visible area along X (landscape)
display_view_depth   = 43.2;   // Visible area along Y (landscape)
display_view_offset_x = 5.0;   // View area offset from PCB left edge
display_view_offset_y = 3.4;   // View area offset from PCB front edge

display_mount_hole_dia = 3.0;  // Mounting hole diameter
display_mount_inset    = 2.5;  // Mounting hole distance from PCB edge

// --- ESP32 DevKitC (38-pin) --------------------------------------------------
esp32_pcb_width   = 28.0;
esp32_pcb_length  = 55.0;
esp32_pcb_thick   = 1.6;
esp32_total_height = 10.0;    // Including components underneath
esp32_usb_width   = 8.0;      // Micro-USB port width
esp32_usb_height  = 3.5;      // Micro-USB port height

// --- TCA9548A I²C Mux Breakout -----------------------------------------------
mux_pcb_width     = 18.0;
mux_pcb_length    = 25.0;
mux_pcb_thick     = 1.6;

// --- Battery (2S LiPo) ------------------------------------------------------
battery_width     = 35.0;     // Typical 2S 1000-1500mAh LiPo
battery_length    = 68.0;
battery_height    = 18.0;
battery_clearance = 2.0;      // Extra space around battery

// --- Ergonomic Grip ----------------------------------------------------------
grip_length       = 80.0;     // Grip length (Z, downward from joystick zone)
grip_width        = 35.0;     // Grip cross-section width
grip_depth        = 30.0;     // Grip cross-section depth
grip_fillet       = 8.0;      // Grip edge rounding

// --- Zone Dimensions ---------------------------------------------------------
// Uniform width for all zones (X axis) – driven by the widest content
zone_width = max(
    joy_center_distance + joy_body_width + 20, // Joystick spacing + margin
    bay_grid_width + 20,                        // Module grid + margin
    display_pcb_width + 20                      // Display PCB + margin
);

// Zone heights (Z axis) – the enclosure shell height
zone_height       = joy_body_height + wall_thickness + 5; // ~42.5mm

// Zone depths (Y axis, front-to-back for each zone)
joy_zone_depth    = joy_body_width + 20;      // ~70mm
module_zone_depth = bay_grid_height + 15;     // ~110mm
display_zone_depth = display_pcb_depth + 15;  // ~65mm

// Total transmitter depth
total_depth = joy_zone_depth + module_zone_depth + display_zone_depth;

// --- Mounting Posts -----------------------------------------------------------
mount_post_dia     = 8.0;     // Outer diameter of mounting post
mount_post_inset_x = 8.0;     // Inset from zone edge (X)
mount_post_inset_y = 8.0;     // Inset from zone edge (Y)

// --- Cable Routing -----------------------------------------------------------
cable_slot_width   = 10.0;    // Width of cable routing slots between zones
cable_slot_height  = 4.0;     // Height of cable routing slots

// --- Colors (for assembly visualization) -------------------------------------
color_joystick     = [0.2, 0.5, 0.8, 0.9];
color_modules      = [0.8, 0.5, 0.2, 0.9];
color_display      = [0.2, 0.8, 0.3, 0.9];
color_electronics  = [0.6, 0.2, 0.6, 0.9];
color_battery      = [0.8, 0.2, 0.2, 0.9];
color_grip         = [0.4, 0.4, 0.4, 0.9];

// --- Debug / Visualization ---------------------------------------------------
$fn = 60;           // Default facet count for curves
explode = 0;        // Explosion distance (0 = assembled, >0 = exploded view)
show_components = true; // Show placeholder components (joysticks, display, etc.)
