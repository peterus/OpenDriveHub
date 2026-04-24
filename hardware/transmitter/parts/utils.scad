// Shared helpers for OpenDriveHub transmitter parts.
// Keep this thin — only generic utilities used across multiple parts.

include <BOSL2/std.scad>
include <parameters.scad>

// ----- Consistent color palette for vitamins (non-printed parts) -----
// Use these in part modules so rendered views are visually coherent.
// Printed parts should use COLOR_PRINTED (typically overridden by slicer anyway).
COLOR_METAL      = "#c0c0c0";   // steel, aluminum, shafts
COLOR_PCB        = "#2a7a2a";   // PCBs
COLOR_PLASTIC    = "#202020";   // ABS/nylon housings
COLOR_RUBBER     = "#404040";   // boots, grommets
COLOR_BRASS      = "#c8a060";   // pin headers, inserts
COLOR_BATTERY    = "#a02020";   // LiPo pouches
COLOR_DISPLAY    = "#101028";   // display glass/active area
COLOR_PRINTED    = "#d8c890";   // printed parts (filament-ish tan)
COLOR_GHOST      = [0.7, 0.7, 0.7, 0.3];  // semi-transparent mockup

// ----- print_bed_check: echo a warning if a part is larger than the bed -----
// Call inside a part module with the part's XY footprint.
// Does not produce geometry; pure diagnostic.
module print_bed_check(size_xy, label="part") {
    fits_x = size_xy.x <= PRINT_BED.x;
    fits_y = size_xy.y <= PRINT_BED.y;
    if (!(fits_x && fits_y)) {
        echo(str("[print_bed_check] WARNING: '", label,
                 "' is ", size_xy.x, "x", size_xy.y,
                 " mm but bed is ", PRINT_BED.x, "x", PRINT_BED.y,
                 " mm — split or rotate."));
    } else {
        echo(str("[print_bed_check] '", label, "' fits (",
                 size_xy.x, "x", size_xy.y, " on ",
                 PRINT_BED.x, "x", PRINT_BED.y, ")."));
    }
}

// ----- explode_shift: position helper for assemblies -----
// Wraps children in a translate along `axis` by EXPLODE * factor.
// EXPLODE is set in parameters.scad (0 = assembled, >0 = separated).
// Use in assembly.scad so one parameter controls the whole explode animation.
module explode_shift(factor=1, axis=UP) {
    translate(axis * EXPLODE * factor) children();
}
