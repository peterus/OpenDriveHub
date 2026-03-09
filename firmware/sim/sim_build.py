"""
sim_build.py – PlatformIO extra_script for the simulation environment.

Adds the simulation shim include path and compiles the shim source files
alongside the project's own sources.
"""

Import("env")

import os

sim_dir = os.path.abspath(os.path.join(env.subst("$PROJECT_DIR"), "sim"))

# Add shim headers to the include path (highest priority).
env.Prepend(CPPPATH=[os.path.join(sim_dir, "include")])

# Compile the shim source files as part of the build.
env.BuildSources(
    os.path.join(env.subst("$BUILD_DIR"), "sim_shim"),
    os.path.join(sim_dir, "src"),
)
