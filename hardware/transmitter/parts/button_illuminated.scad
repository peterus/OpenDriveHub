// Illuminated square panel-mount pushbutton (momentary + single-color LED).
//
// Translucent square cap over a dark bezel, internal LED for backlight,
// mounted through a rectangular panel cutout. Used for shortcut / quick-
// action tiles on the front panel.
//
// IMPORTANT: Dimensions are for a generic PB-12 / PB-22A-style clone.
// Many vendors (Schurter, Apem, countless AliExpress equivalents). Cap and
// bezel sizes vary by a few mm — verify with your chosen part.
//
// Conventions:
//   - Origin: center of attachable bounding volume.
//   - +Z is toward the user (cap face).
//   - Anchor "panel" sits on the panel-mating face (top of body, bottom of
//     bezel-flange that clamps against the panel).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Illuminated pushbutton nominal dimensions (verify) -----
BTN_IL_CAP      = [12, 9, 2.5];    // translucent lens cap [x, y, z]
BTN_IL_BEZEL    = [14, 10, 3];     // dark bezel frame above panel
BTN_IL_BODY     = [14, 10, 10];    // body below panel
BTN_IL_PIN_X    = 5.0;             // pin spacing in X (2 pairs)
BTN_IL_PIN_Y    = 5.0;             // pin spacing in Y (switch vs LED)
BTN_IL_PIN_L    = 4.0;             // pin length below body
BTN_IL_PANEL_CUT = [12.5, 8.5];    // panel cutout WxD (bezel flange covers)
BTN_IL_LED      = "#ff8030";       // lit orange glow (render color)

module button_illuminated(anchor=CENTER, spin=0, orient=UP) {
    total_z = BTN_IL_BODY.z + BTN_IL_PIN_L + BTN_IL_BEZEL.z + BTN_IL_CAP.z;
    size    = [BTN_IL_BEZEL.x, BTN_IL_BEZEL.y, total_z];
    anchors = [
        named_anchor("panel",     [0, 0, -size.z/2 + BTN_IL_BODY.z + BTN_IL_PIN_L], UP),
        named_anchor("terminals", [0, 0, -size.z/2], DOWN),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        down(size.z/2 - BTN_IL_BODY.z - BTN_IL_PIN_L)
            _btn_il_geometry();
        children();
    }
}

module _btn_il_geometry() {
    // Body below panel
    color(COLOR_PLASTIC)
        down(BTN_IL_BODY.z)
            cuboid(BTN_IL_BODY, anchor=BOTTOM);

    // 4 pins (switch pair + LED pair), at corners
    color(COLOR_BRASS)
        down(BTN_IL_BODY.z + BTN_IL_PIN_L/2)
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * BTN_IL_PIN_X/2, sy * BTN_IL_PIN_Y/2, 0])
                    cuboid([0.6, 0.6, BTN_IL_PIN_L]);

    // Bezel (above panel, flange that rests on panel top)
    color("#181818")
        cuboid(BTN_IL_BEZEL, anchor=BOTTOM);

    // Translucent illuminated cap on top of bezel
    color(BTN_IL_LED)
        up(BTN_IL_BEZEL.z)
            cuboid(BTN_IL_CAP, anchor=BOTTOM, rounding=0.5, edges="Z");
}

// -----------------------------------------------------------------------------
// Panel cutout helper: rectangular hole for the body, covered by bezel flange.
// -----------------------------------------------------------------------------
module button_illuminated_panel_cutout(panel_thick=2) {
    translate([0, 0, -0.1])
        cuboid([BTN_IL_PANEL_CUT.x, BTN_IL_PANEL_CUT.y, panel_thick + 0.2],
               anchor=BOTTOM);
}

// -----------------------------------------------------------------------------
// Standalone preview.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=200) button_illuminated();
    else button_illuminated();
}
