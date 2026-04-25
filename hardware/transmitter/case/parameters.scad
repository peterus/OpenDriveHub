// Case-specific constants for the transmitter shell.
//
// Coordinate convention used by all case modules:
//   z = 0   : panel-mating face (= panel inner surface, where component
//             bodies meet the panel from inside the case)
//   z = +Z  : out toward the user (panel material occupies z=[0, PANEL_T])
//   z = -Z  : into the case interior (component bodies live here)
//   x = 0   : panel horizontal center (matches layout_front.scad)
//   y = 0   : panel vertical center (+Y up, -Y down)

include <BOSL2/std.scad>
include <../parts/parameters.scad>   // global $fn, FIT_*, INSERT_M3_*
include <../parts/utils.scad>        // COLOR_*, helpers

// ----- Front-panel reference (must match parts/layout_front.scad) -----
PANEL_W = 280;
PANEL_H = 130;

// ----- Wall and panel thickness -----
WALL_T          = 3;     // outer side-wall thickness
PANEL_T         = 3;     // front-panel thickness (where components mount)

// ----- Top vs bottom shell depth (sum = total case interior depth) -----
// TOP_DEPTH must clear the joystick body (28mm) + pin block (6mm) — pin
// block is allowed to extend a few mm into the bottom shell cavity since
// nothing else lives at that XY position.
// BOTTOM_DEPTH must clear the battery + slot walls + lid (~18mm) plus a
// little headroom.
TOP_DEPTH       = 30;    // top shell rear opening to front panel face
BOTTOM_DEPTH    = 25;    // bottom shell rear face to front opening

// ----- Outer cosmetics -----
// CORNER_R and TOP_EDGE_R should match (or be close) so the vertical
// rounding and the top-face rounding blend smoothly at the corner. Big
// difference between them creates a visible "saddle" seam at the
// front-top corners.
CORNER_R        = 14;    // vertical corner rounding
TOP_EDGE_R      = 2.5;   // top-face edge rounding (must stay below WALL_T=3
                         // or it eats through the panel slab at the corners)
EDGE_FILLET     = 1.5;   // small fillet on inner edges to reduce stress

// ----- Taper: front face bigger than back face for a "shaped" profile -----
// size1 (back) = front dimension - 2*TAPER_*
TAPER_X         = 6;     // case narrows by 12mm total in X from front to back
TAPER_Y         = 4;     // case narrows by 8mm total in Y from front to back

// ----- Outer dimensions (without grips) -----
CASE_W          = PANEL_W + 2*WALL_T;
CASE_H          = PANEL_H + 2*WALL_T;
CASE_DEPTH      = TOP_DEPTH + BOTTOM_DEPTH;

// ----- Mounting bosses for top-bottom screw fastening (M3 + heat-set) -----
BOSS_OD         = 7;
BOSS_INSERT_D   = INSERT_M3_POCKET_D;
BOSS_INSERT_H   = INSERT_M3_POCKET_H;

// 4 case-corner bosses connect top and bottom shells. Insert lives in the
// top-shell boss; screw enters from the back panel exterior, runs through
// a hollow bottom-shell boss, crosses the mating face, threads into the
// insert. Position chosen so the boss merges into the side wall at the
// panel face (smallest section due to taper) for strength while staying
// fully inside the cavity at the mating face.
//
// The top boss is kept short to clear the toggle-switch bodies that hang
// under the front panel into the upper case. The bottom boss runs the
// full bottom-shell depth so the screw is supported all the way.
//
// Screw: M3 x 40 (5mm thread engagement into the insert).
CASE_BOSS_OFFSET_X   = 129;
CASE_BOSS_OFFSET_Y   = 56;
CASE_BOSS_HEIGHT_TOP = TOP_DEPTH - PANEL_T;       // full pillar — anchors at panel
CASE_BOSS_HEIGHT_BOT = BOTTOM_DEPTH - PANEL_T;    // full pillar — anchors at back panel
// Conical foot at the bottom-shell boss base. Widens the moment of inertia
// at the cantilever root so the tower doesn't snap off under lateral load.
CASE_BOSS_FOOT_D     = 10;
CASE_BOSS_FOOT_H     = 4;

// Stiffener fins from boss to the two nearest walls. Two per boss
// (cardinal directions toward the case corner), full boss height. The
// fin is intentionally OVERLENGTH — the shell trims it to the outer
// prismoid via intersection, so the outer face follows the wall taper
// and the rounded corner exactly at every Z level.
CASE_BOSS_FIN_L      = 20;     // overlength, will be clipped to outer wall
CASE_BOSS_FIN_T      = 1.5;

// ----- Battery compartment (LiPo cover access on the bottom-shell back) -----
// Battery is BATT_BODY=[80,35,15]; cover/opening sized with clearance.
// Position is in case-world XY; battery shifted toward the bottom of the
// case for grip-balance (heavier mass closer to the user's grip).
BATT_POS_X        = 0;
BATT_POS_Y        = -25;
BATT_OPENING_W    = 82;     // through-hole for battery insertion (battery + 1mm/side)
BATT_OPENING_H    = 37;
BATT_COVER_W      = 110;    // cover plate footprint (must be wider than the boss row)
BATT_COVER_H      = 64;
BATT_COVER_T      = 2;      // cover plate thickness
BATT_COVER_RECESS = 2;      // nominal recess depth — actual cut = RECESS + Z_GAP,
                            // so the cover top sits BATT_COVER_Z_GAP below flush
BATT_COVER_R      = 2;      // corner rounding on the cover plate
// Print clearance so the cover doesn't bind in the recess and the gap is
// visible in OpenSCAD assembly checks.
BATT_COVER_FIT    = FIT_FREE; // XY clearance per side (slip fit)
BATT_COVER_Z_GAP  = 0.3;      // Z gap — cover top this far below outer panel face

// Corner bosses inside the bottom-shell — each holds two M3 heat-set inserts
// (one near the back-panel for the exterior cover, one at the top for the
// interior battery lid). Boss XY positions clear the battery body with
// ~5mm to each side. Cover screws inset 5mm from cover edge match these.
BATT_BOSS_OD       = 7;     // 1.5mm wall around M3 insert pocket
BATT_BOSS_OFFSET_X = 45;    // = BATT_BODY.x/2 + 5 (battery edge + 5mm)
BATT_BOSS_OFFSET_Y = 22;    // = BATT_BODY.y/2 + 4.5
BATT_BOSS_HEIGHT   = 16;    // boss reaches from back-panel-interior to lid level

// Battery-slot walls — 4 walls between the bosses keep the LiPo from
// sliding around. The +Y wall has a notch where the power + balance
// wires exit toward the main PCB.
BATT_WALL_T        = 2;     // slot wall thickness
BATT_WALL_INSET    = 1;     // gap between battery edge and wall inner face
BATT_WIRE_NOTCH_W  = 22;    // notch width in +Y wall (X axis)
BATT_WIRE_NOTCH_H  = 10;    // notch height (Z axis), measured from boss top down

// Interior battery lid — sits on the boss tops, prevents the LiPo from
// dropping into the case interior.
BATT_LID_W        = BATT_COVER_W;
BATT_LID_H        = BATT_COVER_H;
BATT_LID_T        = 2;
BATT_LID_R        = BATT_COVER_R;

// M3 clearance hole for the screws (slightly oversized for FDM tolerance).
COVER_SCREW_CLEAR = 3.5;

// ----- USB-C extension cable mount (parts/usb_c_extension_cable.scad) -----
// Female panel-mount housing sits AGAINST the outer face of the -Y wall.
// 2 M2 screws come from outside through the wall and thread into M2
// heat-set inserts in bosses on the inner side of the wall. The screws
// trap the housing flange against the panel.
USBC_POS_X         = 80;       // X centre on the -Y wall
USBC_POS_Z         = -11;      // Z position in BOTTOM-SHELL LOCAL coords
                               // (-11 ≈ centre of the 22mm-tall cavity)
USBC_SCREW_PITCH   = 17;       // confirmed
USBC_OPENING_W     = 9.4;      // USB-C cable-entry cutout (with FIT_FREE)
USBC_OPENING_H     = 3.7;
USBC_SCREW_CLEAR   = 2.4;      // M2 clearance hole through wall
USBC_BOSS_OD       = 6;        // M2 insert + 1.5mm wall
USBC_BOSS_H        = INSERT_M2_POCKET_H + 1;  // pocket + 1mm cap behind

// =============================================================================
// Layout positions duplicated from parts/layout_front.scad.
// MUST match the layout file. If you change positions there, update here too.
// =============================================================================
DISPLAY_POS         = [0, 12];

JOYSTICK_X          = 85;
JOYSTICK_Y          = DISPLAY_POS.y;

TOGGLE_Y            = 45;
TOGGLE_SPACING_X    = 18;
TOGGLE_CLUSTER_OFF  = 102;
TOGGLE_LEFT_X       = [-TOGGLE_CLUSTER_OFF - TOGGLE_SPACING_X,
                       -TOGGLE_CLUSTER_OFF,
                       -TOGGLE_CLUSTER_OFF + TOGGLE_SPACING_X];
TOGGLE_RIGHT_X      = [ TOGGLE_CLUSTER_OFF - TOGGLE_SPACING_X,
                        TOGGLE_CLUSTER_OFF,
                        TOGGLE_CLUSTER_OFF + TOGGLE_SPACING_X];

ILLUM_BTN_Y         = -32;
ILLUM_BTN_SPACING   = 18;
ILLUM_BTN_CLUSTER   = JOYSTICK_X;
ILLUM_BTN_LEFT_X    = [-ILLUM_BTN_CLUSTER - ILLUM_BTN_SPACING,
                       -ILLUM_BTN_CLUSTER,
                       -ILLUM_BTN_CLUSTER + ILLUM_BTN_SPACING];
ILLUM_BTN_RIGHT_X   = [ ILLUM_BTN_CLUSTER - ILLUM_BTN_SPACING,
                        ILLUM_BTN_CLUSTER,
                        ILLUM_BTN_CLUSTER + ILLUM_BTN_SPACING];

ENCODER_X           = 30;
ENCODER_Y           = -42;

NAV_BTN_Y           = ENCODER_Y;
NAV_BTN_SPACING     = 10;
