// Battery cover — separate printed part that sits in the recess on the
// bottom-shell back panel and seals the battery compartment.
//
// First iteration: bare plate. Screw holes and an inside foam-pad seat
// come in the next pass once the bosses-with-inserts are wired through.
//
// Print orientation: largest face flat on the bed.

include <BOSL2/std.scad>
include <parameters.scad>

module battery_cover() {
    color(COLOR_PRINTED)
        difference() {
            cuboid(
                [BATT_COVER_W, BATT_COVER_H, BATT_COVER_T],
                rounding=BATT_COVER_R, edges="Z"
            );
            // 4 M3 clearance holes at the boss positions
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*BATT_BOSS_OFFSET_X, sy*BATT_BOSS_OFFSET_Y, 0])
                    cyl(d=COVER_SCREW_CLEAR, l=BATT_COVER_T + 0.2, anchor=CENTER);
        }
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) battery_cover();
