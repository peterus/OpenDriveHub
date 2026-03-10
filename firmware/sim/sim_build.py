# Copyright (C) 2026 Peter Buchegger
#
# This file is part of OpenDriveHub.
#
# OpenDriveHub is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OpenDriveHub is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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
