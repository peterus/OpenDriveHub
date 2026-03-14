# Transmitter Enclosure

3D-printable modular enclosure for the OpenDriveHub transmitter, designed in
[OpenSCAD](https://openscad.org/).

---

## Zone Architecture

The enclosure is built from three zones arranged **horizontally** (front to
back), connected via tongue-and-groove joints and M3 screws. As held by the
user (body → away):

```
[Joystick Zone] → [Module Zone] → [Display Zone]
     front             middle            rear
```

| # | Zone | File | Width | Depth |
|---|------|------|-------|-------|
| 1 | Joystick (2× D400-R4 + grips) | `joystick/joystick_zone.scad` | 195 mm | 70 mm |
| 2 | Module grid (5×3 × 30 mm bays) | `modules/module_zone.scad` | 195 mm | 110 mm |
| 3 | Display (ILI9341 2.8″ landscape) | `display/display_zone.scad` | 195 mm | 65 mm |

All zones share the same **width (~195 mm)** and **height (~42.5 mm)**.
Each zone fits on a **200×200 mm print bed** individually.

Open `assembly.scad` in OpenSCAD to see all zones together. Set
`explode = 30;` in `parameters.scad` for an exploded view.

---

## File Structure

```
enclosure/
├── assembly.scad              ← Full assembly view
├── parameters.scad            ← All dimensions (edit this!)
├── utils.scad                 ← Helper modules + zone connectors
├── joystick/
│   └── joystick_zone.scad     ← Joystick zone with grips + battery
├── modules/
│   └── module_zone.scad       ← Module grid + electronics underneath
├── display/
│   └── display_zone.scad      ← Display bezel (LCD landscape)
└── renders/                   ← Pre-rendered PNG images
```

---

## Key Parameters

All dimensions are defined in `parameters.scad`. Key values:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `wall_thickness` | 2.5 mm | Outer wall thickness |
| `zone_width` | ~195 mm | Uniform width for all zones |
| `zone_height` | ~42.5 mm | Zone shell height |
| `bay_unit_size` | 30 mm | Module bay size |
| `bay_grid_cols` × `bay_grid_rows` | 5 × 3 | Module grid layout |
| `joy_center_distance` | 125 mm | Joystick center-to-center |
| `tongue_width` | 15 mm | Tongue-and-groove joint width |
| `print_bed_x/y` | 200 mm | Max print bed dimensions |
| `explode` | 0 | Exploded view distance |

---

## Bill of Materials (Hardware)

| Qty | Item | Purpose |
|-----|------|---------|
| ~12 | M3×8 socket head cap screws | Zone-to-zone connection |
| ~12 | M3×5×5 brass heat-set inserts | Threaded mounting points |
| 15 | M3 screws (short) | Module retention (rear) |
| 4 | M3 screws (various lengths) | Display PCB mounting |
| 1 | Power switch (12×8 mm) | Battery on/off |
| 1 | XT30 connector pair | Battery connection |

---

## Printing Guidelines

- **Material:** PLA or PETG (PETG recommended for durability)
- **Layer height:** 0.2 mm
- **Infill:** 20–30% (gyroid or grid)
- **Walls:** 3 perimeters minimum
- **Supports:** Only needed for grip sections
- **Print bed:** Each zone fits on 200×200 mm individually
- **Orientation:** Print each zone upside-down (open top facing build plate)

---

## Assembly

1. Print all three zones
2. Install heat-set inserts into mounting points
3. Mount ESP32 and TCA9548A into the module zone (underneath grid)
4. Slide zones together via tongue-and-groove joints
5. Secure with M3 screws through the joint edges
6. Insert LCD into the display zone bezel
7. Insert battery into the joystick zone compartment
8. Mount joysticks from the top (panel-mount)

---

## Customization

Edit `parameters.scad` to adapt the design:

- **Different joysticks:** Adjust `joy_*` parameters
- **Fewer/more module bays:** Change `bay_grid_cols` and `bay_grid_rows`
- **Larger display:** Adjust `display_*` parameters
- **Different battery:** Adjust `battery_*` parameters
- **Thicker walls:** Increase `wall_thickness`

After changing parameters, open `assembly.scad` to verify the design and
check the console output for dimension/fit warnings.

---

## License

This hardware design is licensed under GPL-3.0-or-later, consistent with the
OpenDriveHub project. See [LICENSE](../../../LICENSE).
