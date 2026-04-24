// Long-bat miniature toggle switch, M6 bushing.
//
// Electrical variant (2-pos ON-ON, 3-pos ON-OFF-ON, momentary) is not
// encoded here — outer dimensions and panel cutout are identical across
// all common variants. Exact mix is decided at BOM / firmware time.
//
// IMPORTANT: Body/bushing dimensions vary between manufacturers. These
// numbers fit APEM 5236-style / Carling M-series mini-toggles roughly.
// Verify against your specific switch with a caliper.
//
// Conventions:
//   - Origin: center of the part's overall bounding volume (attachable default).
//   - +Z points along the lever when centered.
//   - Anchor "panel" sits on the panel-mating face (top of body).
//   - Anchor "terminals" sits below the solder lugs.
//   - The `tilt` parameter rotates the lever around X; useful to preview
//     the switch in a thrown position.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Long-bat toggle dimensions (verify) -----
SW_BODY      = [13, 8, 13];     // body below panel [x, y, z]
SW_BUSH_D    = 6.35;             // M6-ish threaded bushing OD
SW_BUSH_H    = 9;                // bushing length above body (fits panel + nut + washer)
SW_NUT_AF    = 9;                // hex panel nut across-flats
SW_NUT_H     = 2.5;              // hex nut thickness
SW_LEVER_D   = 2.5;              // lever shaft OD
SW_LEVER_L   = 24;               // long-bat total length (bushing-top to ball-tip)
SW_BALL_D    = 4;                // ball at lever tip
SW_TERM_L    = 5;                // solder-lug length below body
SW_TERM_PITCH= 4;                // spacing between terminal lugs (SPDT = 3 in a row)
SW_TILT_DEG  = 0;                // lever tilt (0 = centered, typical throw ±30°)
SW_PANEL_CUT = 6.5;              // panel cutout Ø (bushing + clearance)

module toggle_longbat(anchor=CENTER, spin=0, orient=UP, tilt=SW_TILT_DEG) {
    // SW_LEVER_L already measures bushing-top to ball-tip (ball embedded in top).
    total_z = SW_BODY.z + SW_TERM_L + SW_BUSH_H + SW_LEVER_L;
    nut_od  = SW_NUT_AF / cos(30);  // hex circumscribed Ø from across-flats
    size = [max(SW_BODY.x, nut_od), max(SW_BODY.y, nut_od), total_z];

    anchors = [
        named_anchor("panel",     [0, 0, -size.z/2 + SW_BODY.z + SW_TERM_L], UP),
        named_anchor("terminals", [0, 0, -size.z/2], DOWN),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        down(size.z/2 - SW_BODY.z - SW_TERM_L)
            _toggle_geometry(tilt);
        children();
    }
}

module _toggle_geometry(tilt) {
    // Body (below panel)
    color(COLOR_PLASTIC)
        down(SW_BODY.z)
            cuboid(SW_BODY, anchor=BOTTOM);

    // Solder lugs (3 in a row for SPDT)
    color(COLOR_BRASS)
        down(SW_BODY.z + SW_TERM_L/2)
            for (x = [-SW_TERM_PITCH, 0, SW_TERM_PITCH])
                translate([x, 0, 0])
                    cuboid([1.5, 4, SW_TERM_L]);

    // Threaded bushing (above body, passes through panel)
    color(COLOR_METAL)
        cyl(d=SW_BUSH_D, l=SW_BUSH_H, anchor=BOTTOM);

    // Hex panel nut (shown sitting on body-top for clarity; in real use it's
    // above the panel).
    color(COLOR_METAL)
        up(SW_NUT_H/2)
            zrot(30)
                cyl(d=SW_NUT_AF / cos(30), l=SW_NUT_H, $fn=6, anchor=CENTER);

    // Lever and ball — tilt rotates them around X
    up(SW_BUSH_H)
        xrot(tilt) {
            color(COLOR_METAL)
                cyl(d=SW_LEVER_D, l=SW_LEVER_L - SW_BALL_D/2, anchor=BOTTOM);
            color(COLOR_METAL)
                up(SW_LEVER_L - SW_BALL_D/2)
                    sphere(d=SW_BALL_D);
        }
}

// -----------------------------------------------------------------------------
// Panel cutout helper: difference from panel to mount the switch.
// Centered on the bushing axis, origin at panel-top face.
// -----------------------------------------------------------------------------
module toggle_longbat_panel_cutout(panel_thick=2) {
    translate([0, 0, -0.1])
        cyl(d=SW_PANEL_CUT, l=panel_thick + 0.2, anchor=BOTTOM);
}

// -----------------------------------------------------------------------------
// Standalone preview — override SHOW_* or tilt via MCP `variables`.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;
PREVIEW_TILT    = 0;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=200) toggle_longbat(tilt=PREVIEW_TILT);
    else toggle_longbat(tilt=PREVIEW_TILT);
}
