// Global parameters for OpenDriveHub transmitter hardware.
// Every dimension used in parts/*.scad lives here. Derive; don't duplicate.

// --- Rendering quality ---
$fn = 64;

// --- Printer constraints ---
PRINT_BED      = [200, 200];   // primary printer; 220x220 and 300x300 also available
NOZZLE_DIAM    = 0.4;
MIN_WALL       = 1.2;          // 3 perimeters @ 0.4mm
LAYER_H        = 0.2;

// --- Fit tolerances (FDM, tune per printer) ---
FIT_SLIP       = 0.2;          // radial offset for slip fit
FIT_PRESS      = -0.08;        // radial offset for press fit
FIT_FREE       = 0.35;         // radial offset for free/clearance

// --- Heat-set insert pocket (M3, 4mm OD, 4mm length) ---
INSERT_M3_OD   = 4.0;
INSERT_M3_LEN  = 4.0;
INSERT_M3_POCKET_D = INSERT_M3_OD - 0.1;
INSERT_M3_POCKET_H = INSERT_M3_LEN + 0.5;

// --- Assembly visualization ---
EXPLODE        = 0;            // 0 = assembled; >0 separates parts along Z for preview

// --- 2S LiPo pouch nominal dimensions (verify with calipers!) ---
// Used by parts/battery.scad and case/bottom_shell.scad battery compartment.
BATT_BODY      = [80, 35, 15]; // pouch body [x, y, z] in mm
