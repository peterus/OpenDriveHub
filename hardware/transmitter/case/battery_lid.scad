// Battery interior lid — separate printed part that seals the top of the
// battery slot from inside the case. Screws into the M3 inserts at the
// top of the four corner bosses in bottom_shell.
//
// Without this lid, the battery could fall into the case interior once
// the exterior cover is removed (or when the case is upside-down).
//
// Print orientation: largest face flat on the bed (lid sits face-down).

include <BOSL2/std.scad>
include <parameters.scad>

module battery_lid() {
    color(COLOR_PRINTED)
        difference() {
            cuboid(
                [BATT_LID_W, BATT_LID_H, BATT_LID_T],
                rounding=BATT_LID_R, edges="Z"
            );
            // 4 M3 clearance holes at the boss positions
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*BATT_BOSS_OFFSET_X, sy*BATT_BOSS_OFFSET_Y, 0])
                    cyl(d=COVER_SCREW_CLEAR, l=BATT_LID_T + 0.2, anchor=CENTER);
        }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) battery_lid();
