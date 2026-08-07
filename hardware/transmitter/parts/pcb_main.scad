// Main controller PCB — ESP32-WROOM-32 + charger + ADC + I/O expanders +
// audio amp + USB-C, all in one. Sits behind the display in the case so
// the display module can plug directly into a 22-pin header on the top
// edge.
//
// IMPORTANT: this is a placeholder geometry — outline, mount-hole pattern,
// and connector positions are sketch-quality. Real layout comes from the
// KiCad design once schematic + footprints are settled.
//
// Conventions:
//   - Origin: PCB centroid, on the PCB top face (component-mating side).
//   - PCB extends in -Z by PCB_THICK.
//   - +Y = direction of display (display sits on this edge).
//   - +X = direction of USB-C cutout.
//   - Anchor "display_header" at the female pin-header where the display
//     module plugs in.
//   - Anchor "usb_c_edge" at the USB-C cutout (case-mating edge).

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>
use <pcb_subpanel.scad>  // pulls in PCB_THICK constant

// ----- Main PCB outline -----
MAIN_PCB        = [80, 100];     // [x, y] PCB outline
MAIN_PCB_THICK  = 1.6;
MAIN_MOUNT_INSET = 4;
MAIN_MOUNT_D    = 2.5;

// ----- Display pin header (female, on top edge in +Y) -----
DSP_HDR_PINS    = 22;
DSP_HDR_PITCH   = 2.54;
DSP_HDR_LEN     = DSP_HDR_PINS * DSP_HDR_PITCH;   // ≈ 55.9 mm
DSP_HDR_OFFSET  = MAIN_PCB.y / 2 - 5;             // 5 mm from top edge
DSP_HDR_SIZE    = [DSP_HDR_LEN, 2.54, 8.5];       // housing height ~8.5mm

// ----- USB-C cutout on +X edge -----
USBC_OFFSET_Y   = -MAIN_PCB.y / 2 + 15;           // toward bottom of PCB
USBC_CUTOUT     = [9, 3.5];                       // panel cutout footprint

// ----- ESP32-WROOM module footprint -----
// PCB-trace antenna sits on one short edge — keep it pointing toward the
// case wall, NOT toward the battery or display backlight metalisation.
ESP32_FOOTPRINT = [18, 28];                       // module outline
ESP32_OFFSET    = [-MAIN_PCB.x/2 + 12, -MAIN_PCB.y/2 + 22];
ESP32_HEIGHT    = 3.0;

// ----- Connector positions (5× sub-panel, 2× joystick, 1× battery,
//       1× speaker) — JST-XH housings on PCB top side -----
JST_2P = [8,  4, 5];   // 2-pin (speaker, battery main)
JST_3P = [10, 4, 5];   // 3-pin (battery balance)
JST_4P = [12, 4, 5];   // 4-pin (sub-panel I2C, joystick)

module pcb_main(anchor=CENTER, spin=0, orient=UP) {
    // Total Z range: PCB bottom (-MAIN_PCB_THICK) up to display-header top
    // (+DSP_HDR_SIZE.z). Display header is the tallest component so it
    // dominates the upper bound.
    size = [MAIN_PCB.x, MAIN_PCB.y, MAIN_PCB_THICK + DSP_HDR_SIZE.z];
    pcb_top_z = -size.z/2 + MAIN_PCB_THICK;
    anchors = [
        named_anchor("display_header",
                     [0, DSP_HDR_OFFSET, pcb_top_z + DSP_HDR_SIZE.z],
                     UP),
        named_anchor("usb_c_edge",
                     [MAIN_PCB.x/2, USBC_OFFSET_Y, pcb_top_z - MAIN_PCB_THICK/2],
                     RIGHT),
    ];
    attachable(anchor, spin, orient, size=size, anchors=anchors) {
        down(size.z/2 - MAIN_PCB_THICK)
            _pcb_main_geometry();
        children();
    }
}

module _pcb_main_geometry() {
    // PCB itself, top face at z=0
    color(COLOR_PCB)
        difference() {
            cuboid([MAIN_PCB.x, MAIN_PCB.y, MAIN_PCB_THICK], anchor=TOP);
            // 4 corner mount holes
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*(MAIN_PCB.x/2 - MAIN_MOUNT_INSET),
                           sy*(MAIN_PCB.y/2 - MAIN_MOUNT_INSET),
                           0.1])
                    cyl(d=MAIN_MOUNT_D, l=MAIN_PCB_THICK + 0.4, anchor=TOP);
            // USB-C cutout on +X edge
            translate([MAIN_PCB.x/2 - USBC_CUTOUT.x/2 + 0.1, USBC_OFFSET_Y, 0.1])
                cuboid([USBC_CUTOUT.x + 0.2, USBC_CUTOUT.y, MAIN_PCB_THICK + 0.4],
                       anchor=TOP);
        }

    // Display pin header (female socket) on +Y edge
    color("#101010")
        translate([0, DSP_HDR_OFFSET, DSP_HDR_SIZE.z/2])
            cuboid(DSP_HDR_SIZE);

    // ESP32-WROOM-32 module silhouette (with PCB-trace antenna pointing -X)
    color(COLOR_PLASTIC)
        translate([ESP32_OFFSET.x, ESP32_OFFSET.y, ESP32_HEIGHT/2])
            cuboid([ESP32_FOOTPRINT.x, ESP32_FOOTPRINT.y, ESP32_HEIGHT]);

    // JST connectors — sub-panel cluster (5 of them) along -Y edge
    for (i = [0 : 4]) {
        x = -MAIN_PCB.x/2 + 12 + i * 14;
        color(COLOR_BRASS)
            translate([x, -MAIN_PCB.y/2 + 4, JST_4P.z/2])
                cuboid(JST_4P);
    }

    // Joystick connectors (2x JST-4P) — left edge
    for (i = [0, 1])
        color(COLOR_BRASS)
            translate([-MAIN_PCB.x/2 + 8, MAIN_PCB.y/2 - 30 - i*14, JST_4P.z/2])
                cuboid(JST_4P);

    // Battery connector (3-pin balance + 2-pin power) — bottom-right
    color("#a02020")
        translate([MAIN_PCB.x/2 - 8, -MAIN_PCB.y/2 + 30, JST_3P.z/2])
            cuboid(JST_3P);
    color("#a02020")
        translate([MAIN_PCB.x/2 - 8, -MAIN_PCB.y/2 + 38, JST_2P.z/2])
            cuboid(JST_2P);

    // Speaker connector (2-pin) — bottom-left
    color(COLOR_BRASS)
        translate([-MAIN_PCB.x/2 + 8, -MAIN_PCB.y/2 + 30, JST_2P.z/2])
            cuboid(JST_2P);
}

// =============================================================================
// Standalone preview.
// =============================================================================
SHOW_STANDALONE = true;

if (SHOW_STANDALONE) pcb_main();
