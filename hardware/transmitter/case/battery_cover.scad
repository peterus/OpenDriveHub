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
        cuboid(
            [BATT_COVER_W, BATT_COVER_H, BATT_COVER_T],
            rounding=BATT_COVER_R, edges="Z"
        );
}

// =============================================================================
// Standalone preview
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) battery_cover();
