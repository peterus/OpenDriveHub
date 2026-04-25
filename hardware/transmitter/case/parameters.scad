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
TOP_DEPTH       = 35;    // top shell rear opening to front panel face
BOTTOM_DEPTH    = 35;    // bottom shell rear face to front opening

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
