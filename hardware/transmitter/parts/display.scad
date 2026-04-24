// 4.0" IPS TFT display module, ST7796 driver, 480x320, capacitive touch (FT6236).
//
// Generic hobby-style SPI module with capacitive touch overlay. The module
// consists of a PCB with the driver + touch controller, the LCD glass stack,
// and an FPC ribbon or pin header exiting one edge.
//
// IMPORTANT: Specific boards vary by several mm in PCB outline, active area,
// mounting-hole pattern, and cable exit side. Verify against your actual
// purchased module before finalizing the case panel cutout.
//
// Conventions:
//   - Origin: center of LCD glass face (the user-facing surface).
//   - +Z is out of the screen toward the user.
//   - Anchor "panel" sits on the glass face (panel-mating side).
//   - Anchor "pcb_back" sits on the rear PCB face (where it mounts to standoffs).
//   - Anchor "mount_{fl,fr,bl,br}" at each corner screw hole, on the PCB rear face.

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- 4.0" display module nominal dimensions (verify against exact part) -----
DSP_PCB        = [98, 62, 1.6];    // PCB outline [x, y, z]
DSP_MOD        = [86, 58, 4.5];    // LCD+bezel stack on top of PCB
DSP_ACTIVE     = [84, 56];         // illuminated active area (rendered as lit zone)
DSP_MOUNT_PCD  = [91.5, 55.5];     // corner-to-corner mount hole pattern
DSP_MOUNT_D    = 3.2;              // M3 clearance hole
DSP_PIN_OFFSET = -DSP_PCB.y/2 + 3; // where pin header exits (bottom edge)
DSP_PIN_STRIP  = [30, 3, 8];       // rough pin-header volume [x, y, z below PCB]

module display_4in_ips(anchor=CENTER, spin=0, orient=UP) {
    // Envelope: PCB footprint × (PCB thick + LCD module thick + pin strip below)
    total_z = DSP_PCB.z + DSP_MOD.z + DSP_PIN_STRIP.z;
    size    = [DSP_PCB.x, DSP_PCB.y, total_z];
    // Panel face (glass top) Z in attachable coords:
    panel_z = -size.z/2 + DSP_PIN_STRIP.z + DSP_PCB.z + DSP_MOD.z;

    anchors = [
        named_anchor("panel",    [0, 0, panel_z], UP),
        named_anchor("pcb_back", [0, 0, -size.z/2 + DSP_PIN_STRIP.z], DOWN),
        // Corner mount anchors on PCB rear face
        named_anchor("mount_fl", [-DSP_MOUNT_PCD.x/2, -DSP_MOUNT_PCD.y/2,
                                  -size.z/2 + DSP_PIN_STRIP.z], DOWN),
        named_anchor("mount_fr", [ DSP_MOUNT_PCD.x/2, -DSP_MOUNT_PCD.y/2,
                                  -size.z/2 + DSP_PIN_STRIP.z], DOWN),
        named_anchor("mount_bl", [-DSP_MOUNT_PCD.x/2,  DSP_MOUNT_PCD.y/2,
                                  -size.z/2 + DSP_PIN_STRIP.z], DOWN),
        named_anchor("mount_br", [ DSP_MOUNT_PCD.x/2,  DSP_MOUNT_PCD.y/2,
                                  -size.z/2 + DSP_PIN_STRIP.z], DOWN),
    ];

    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        // Shift geometry so glass top lands at panel_z
        down(size.z/2 - DSP_PIN_STRIP.z - DSP_PCB.z - DSP_MOD.z)
            _display_geometry();
        children();
    }
}

module _display_geometry() {
    // Pin header strip (below PCB)
    color(COLOR_PLASTIC)
        translate([0, DSP_PIN_OFFSET, -DSP_PIN_STRIP.z/2])
            cuboid(DSP_PIN_STRIP);

    // PCB (green) with mounting holes cut through
    color(COLOR_PCB)
        difference() {
            cuboid(DSP_PCB, anchor=BOTTOM);
            // 4 mounting holes
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*DSP_MOUNT_PCD.x/2, sy*DSP_MOUNT_PCD.y/2, -0.1])
                    cyl(d=DSP_MOUNT_D, l=DSP_PCB.z + 0.2, anchor=BOTTOM);
        }

    // LCD module stack on top of PCB, centered. Bezel is a solid frame with
    // the active-area window milled in; the active layer fills the window.
    up(DSP_PCB.z) {
        // Bezel frame with window
        color(COLOR_PLASTIC)
            difference() {
                cuboid(DSP_MOD, anchor=BOTTOM);
                translate([0, 0, DSP_MOD.z - 0.3])
                    cuboid([DSP_ACTIVE.x, DSP_ACTIVE.y, 0.6], anchor=BOTTOM);
            }
        // Active display area fills the window, flush with bezel top
        color(COLOR_DISPLAY)
            translate([0, 0, DSP_MOD.z - 0.3])
                cuboid([DSP_ACTIVE.x, DSP_ACTIVE.y, 0.3], anchor=BOTTOM);
    }
}

// -----------------------------------------------------------------------------
// Panel cutout helper: window for the active display + 4 M3 bolt clearance
// holes matching the mount PCD. Origin at glass-face (panel top).
// -----------------------------------------------------------------------------
module display_4in_ips_panel_cutout(panel_thick=2, window_margin=1.0, bolt_clear=true) {
    // Window — slightly bigger than active area so the bezel is the visible
    // boundary (case hides bezel edges by `window_margin`).
    translate([0, 0, -0.1])
        cuboid([DSP_ACTIVE.x + 2*window_margin,
                DSP_ACTIVE.y + 2*window_margin,
                panel_thick + 0.2], anchor=BOTTOM);

    if (bolt_clear)
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*DSP_MOUNT_PCD.x/2, sy*DSP_MOUNT_PCD.y/2, -0.1])
                cyl(d=DSP_MOUNT_D + 2*FIT_FREE, l=panel_thick + 0.2, anchor=BOTTOM);
}

// -----------------------------------------------------------------------------
// Standalone preview.
// -----------------------------------------------------------------------------
SHOW_STANDALONE = true;
SHOW_SECTION    = false;

if (SHOW_STANDALONE) {
    if (SHOW_SECTION) back_half(s=200) display_4in_ips();
    else display_4in_ips();
}
