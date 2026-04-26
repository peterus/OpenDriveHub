#!/usr/bin/env freecadcmd
"""Convert a STEP file to STL via FreeCAD's headless CLI.

freecadcmd treats every CLI arg as a script file, so we read input/output
paths from environment variables instead of sys.argv.

Usage:
    STEP_IN=path/to/input.step STL_OUT=path/to/output.stl \
        [STL_DEFLECTION=0.1] freecadcmd step_to_stl.py

Linear deflection controls mesh fineness (smaller = more triangles).
0.1 mm is good for PCB-scale fidelity in case-fit visualization.
"""
import os, sys
import FreeCAD, Part, Mesh

step_path = os.environ.get('STEP_IN')
stl_path  = os.environ.get('STL_OUT')
deflection = float(os.environ.get('STL_DEFLECTION', '0.1'))

if not step_path or not stl_path:
    print("ERROR: set STEP_IN and STL_OUT environment variables")
    sys.exit(2)
if not os.path.isfile(step_path):
    print(f"ERROR: input not found: {step_path}")
    sys.exit(1)

print(f"Loading {step_path}")
shape = Part.Shape()
shape.read(step_path)
n_solids_in = len(shape.Solids)
print(f"Shape loaded: {n_solids_in} solids, {len(shape.Faces)} faces")

# Fuse all solids into one before tessellating. KiCad's STEP export keeps the
# PCB body, IC bodies, connectors, etc. as separate solids that *touch* at
# shared faces (SMD pad bottom == PCB top). OpenSCAD's CGAL backend (F6)
# rejects such touch-but-not-overlap configurations as non-manifold and
# silently drops most of the geometry. Fusing into a single closed solid
# eliminates the shared faces and lets F6 render the import correctly.
if n_solids_in > 1:
    print(f"Fusing {n_solids_in} solids into one closed shell")
    fused = shape.Solids[0]
    for s in shape.Solids[1:]:
        fused = fused.fuse(s)
    shape = fused
    print(f"Fused: {len(shape.Solids)} solids, {len(shape.Faces)} faces")

mesh = Mesh.Mesh()
print(f"Tessellating with linear deflection = {deflection} mm")
mesh.addFacets(shape.tessellate(deflection))
print(f"Mesh: {mesh.CountFacets} facets, {mesh.CountPoints} vertices")

mesh.write(stl_path)
size = os.path.getsize(stl_path)
print(f"Wrote {stl_path} ({size:,} bytes)")
