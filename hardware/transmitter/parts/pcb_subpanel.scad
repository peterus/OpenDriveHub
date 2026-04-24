// Front-panel sub-PCBs — small bare-PCB outlines hosting one component group
// each. Seven PCBs total: 2x toggle (L/R), 2x illuminated (L/R), 2x encoder
// (1 each, kept small so the central nav PCB can sit in front without
// physical conflict between tact pins and encoder PCB top), 1x nav-tact.
//
// Each sub-PCB carries a PCF8574 I/O-expander (or similar) on its back
// side, so the cable to the main PCB is just 4 wires (SDA, SCL, VCC, GND)
// via a JST-XH connector at one edge.
//
// Conventions:
//   - Origin: PCB centroid, on the PCB top face (component-mating side).
//   - PCB extends in -Z below origin (just the 1.6 mm board).
//   - Long axis = X. JST connector by default exits the +Y edge ("back").

include <BOSL2/std.scad>
include <parameters.scad>
include <utils.scad>

// ----- Common PCB constants -----
PCB_THICK         = 1.6;
PCB_MOUNT_D       = 2.5;     // M2.5 clearance hole
PCB_MOUNT_INSET   = 3;       // mount-hole inset from PCB edge
PCB_JST_PIN_COUNT = 4;       // I2C: VCC + GND + SDA + SCL
PCB_JST_HOUSING   = [10, 4, 5];  // JST-XH 4-pin housing (rough)
PCB_JST_INSET     = 2;       // distance from PCB edge to connector center

// ----- Per-variant outlines -----
TOGGLE_PCB_SIZE   = [60, 24];   // 3 toggles in a row at 18mm pitch + margin
ILLUM_PCB_SIZE    = [60, 24];   // 3 illuminated buttons at 18mm pitch + margin
ENCODER_PCB_SIZE  = [30, 22];   // 1 encoder + IC + JST per board
NAV_PCB_SIZE      = [32, 16];   // 3 nav tacts at -10/0/+10

// =============================================================================
// Generic helper — draws a sub-PCB with mount holes and a JST connector.
// =============================================================================
module _subpanel_pcb(size_xy, jst_edge="back") {
    pcb = [size_xy.x, size_xy.y, PCB_THICK];

    // Board (top face at z=0)
    color(COLOR_PCB)
        difference() {
            cuboid(pcb, anchor=TOP);
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx*(pcb.x/2 - PCB_MOUNT_INSET),
                           sy*(pcb.y/2 - PCB_MOUNT_INSET),
                           0.1])
                    cyl(d=PCB_MOUNT_D, l=PCB_THICK + 0.4, anchor=TOP);
        }

    // JST connector body (sits on back side of PCB, sticking down -Z)
    jst_y = (jst_edge == "back")  ?  pcb.y/2 - PCB_JST_INSET :
            (jst_edge == "front") ? -pcb.y/2 + PCB_JST_INSET :
                                     0;
    color("#f0f0f0")
        translate([0, jst_y, -PCB_THICK - PCB_JST_HOUSING.z/2])
            cuboid(PCB_JST_HOUSING);
}

// =============================================================================
// Named variants — used by layout_front and (later) the case shell.
// =============================================================================
module subpanel_pcb_toggle3(jst_edge="back") {
    _subpanel_pcb(TOGGLE_PCB_SIZE, jst_edge);
}

module subpanel_pcb_illum3(jst_edge="back") {
    _subpanel_pcb(ILLUM_PCB_SIZE, jst_edge);
}

module subpanel_pcb_encoder1(jst_edge="back") {
    _subpanel_pcb(ENCODER_PCB_SIZE, jst_edge);
}

module subpanel_pcb_nav3(jst_edge="back") {
    _subpanel_pcb(NAV_PCB_SIZE, jst_edge);
}

// =============================================================================
// Standalone preview — shows all three variants side by side.
// =============================================================================
SHOW_STANDALONE = true;

// Pick which variant to preview when opening this file directly.
PREVIEW_VARIANT = "encoder1";   // "toggle3" | "illum3" | "encoder1" | "nav3"

if (SHOW_STANDALONE) {
    if      (PREVIEW_VARIANT == "toggle3")  subpanel_pcb_toggle3();
    else if (PREVIEW_VARIANT == "illum3")   subpanel_pcb_illum3();
    else if (PREVIEW_VARIANT == "encoder1") subpanel_pcb_encoder1();
    else                                     subpanel_pcb_nav3();
}
