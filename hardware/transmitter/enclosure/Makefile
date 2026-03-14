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

# =============================================================================
# OpenDriveHub Transmitter Enclosure – Build System
# =============================================================================
#
# Auto-discovers .scad part files and generates PNG render / STL export targets.
# Adding a new .scad file automatically makes it available – no edits needed.
#
# Prerequisites:
#   - OpenSCAD (command-line)
#   - DISPLAY environment variable (Xvfb for headless systems)
#
# Quick start:
#   make help              Show all targets and options
#   make renders           Render every part in every view
#   make -j$(nproc) renders  Parallel rendering
#   make stl               Export all parts as STL
# =============================================================================

# Disable built-in rules for faster parsing
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

# ─── Configuration (override via command line or environment) ─────────────────

OPENSCAD     ?= openscad
IMG_WIDTH    ?= 1920
IMG_HEIGHT   ?= 1080
COLORSCHEME  ?= Cornfield
FN           ?= 60
RENDER_DIR   ?= renders
EXPLODE      ?= 30
PNG_MODE     ?= --render

# ─── Views & Camera Angles ───────────────────────────────────────────────────
#
# Each view is defined by a camera rotation (rot_x,rot_y,rot_z) and a
# projection type (p=perspective, o=orthographic).
# --autocenter and --viewall handle translation and distance automatically.
#
# To add a custom view, define CAM_<name> and PROJ_<name>, then add <name>
# to the VIEWS list.

VIEWS ?= iso_front iso_back top bottom front back left right

CAM_iso_front := 55,0,25
CAM_iso_back  := 55,0,205
CAM_top       := 0,0,0
CAM_bottom    := 180,0,0
CAM_front     := 90,0,0
CAM_back      := 90,0,180
CAM_left      := 90,0,90
CAM_right     := 90,0,270

PROJ_iso_front := p
PROJ_iso_back  := p
PROJ_top       := o
PROJ_bottom    := o
PROJ_front     := o
PROJ_back      := o
PROJ_left      := o
PROJ_right     := o

# ─── Auto-Discovery ──────────────────────────────────────────────────────────
#
# Library files are included/used by other files and produce no geometry on
# their own.  Everything else is treated as a renderable part.
# Stems (filename without .scad) must be unique across subdirectories.

SCAD_EXCLUDE := parameters.scad utils.scad

SCAD_PARTS := $(shell find . -name '*.scad' \
    $(foreach x,$(SCAD_EXCLUDE),! -name '$(x)') \
    ! -path './$(RENDER_DIR)/*' \
    -printf '%P\n' | sort)

# Common dependencies – a change here rebuilds everything
SCAD_DEPS := $(wildcard parameters.scad utils.scad)

# Unique part stems (filename without extension)
STEMS := $(sort $(foreach p,$(SCAD_PARTS),$(basename $(notdir $(p)))))

# Source map: SRCMAP_<stem> → relative path to .scad file
$(foreach p,$(SCAD_PARTS),$(eval SRCMAP_$(basename $(notdir $(p))) := $(p)))

# ─── Generated Target Lists ──────────────────────────────────────────────────

ALL_PNGS := $(foreach s,$(STEMS),$(foreach v,$(VIEWS),$(RENDER_DIR)/$(s)/$(v).png))
ALL_STLS := $(foreach s,$(STEMS),$(RENDER_DIR)/$(s)/$(s).stl)

EXPLODED_VIEWS := iso_front iso_back
EXPLODED_PNGS  := $(foreach v,$(EXPLODED_VIEWS),$(RENDER_DIR)/assembly/exploded_$(v).png)

# ─── Phony Targets ───────────────────────────────────────────────────────────

.PHONY: all renders stl assembly-exploded \
        clean clean-all list-parts help \
        $(foreach s,$(STEMS),render-$(s) stl-$(s))

# ─── Main Targets ────────────────────────────────────────────────────────────

all: renders

renders: $(ALL_PNGS)
	@echo ""
	@echo "✓ All $(words $(ALL_PNGS)) renders complete in $(RENDER_DIR)/."

stl: $(ALL_STLS)
	@echo ""
	@echo "✓ All $(words $(ALL_STLS)) STL exports complete in $(RENDER_DIR)/."

assembly-exploded: $(EXPLODED_PNGS)
	@echo ""
	@echo "✓ Exploded assembly views complete."

# ─── Per-Part Convenience Targets ─────────────────────────────────────────────
# Usage: make render-assembly, make stl-joystick_zone, etc.

$(foreach s,$(STEMS),$(eval render-$(s): $(foreach v,$(VIEWS),$(RENDER_DIR)/$(s)/$(v).png)))
$(foreach s,$(STEMS),$(eval stl-$(s): $(RENDER_DIR)/$(s)/$(s).stl))

# ─── Build Rules ──────────────────────────────────────────────────────────────
#
# Uses .SECONDEXPANSION so that the implicit rule can resolve the source .scad
# file from the target path at prerequisite-expansion time:
#
#   Target:  renders/<stem>/<view>.png
#   $* =     <stem>/<view>
#   word 1 = <stem>  →  SRCMAP lookup  →  source .scad path
#   word 2 = <view>  →  CAM / PROJ lookup

.SECONDEXPANSION:

# --- PNG render rule ---
$(RENDER_DIR)/%.png: $$(SRCMAP_$$(word 1,$$(subst /, ,$$*))) $$(SCAD_DEPS)
	@mkdir -p $(dir $@)
	@echo "[RENDER] $*"
	@$(OPENSCAD) $(PNG_MODE) --autocenter --viewall \
		--imgsize=$(IMG_WIDTH),$(IMG_HEIGHT) \
		--colorscheme=$(COLORSCHEME) \
		--camera=0,0,0,$(CAM_$(word 2,$(subst /, ,$*))),0 \
		--projection=$(PROJ_$(word 2,$(subst /, ,$*))) \
		-D '$$fn=$(FN)' \
		-o $@ $<

# --- STL export rule ---
$(RENDER_DIR)/%.stl: $$(SRCMAP_$$(word 1,$$(subst /, ,$$*))) $$(SCAD_DEPS)
	@mkdir -p $(dir $@)
	@echo "[STL] $(word 1,$(subst /, ,$*))"
	@$(OPENSCAD) -D '$$fn=$(FN)' -o $@ $<

# --- Exploded assembly (special case with explode parameter) ---
$(RENDER_DIR)/assembly/exploded_%.png: assembly.scad $$(SCAD_DEPS)
	@mkdir -p $(dir $@)
	@echo "[RENDER] assembly/exploded_$*"
	@$(OPENSCAD) $(PNG_MODE) --autocenter --viewall \
		--imgsize=$(IMG_WIDTH),$(IMG_HEIGHT) \
		--colorscheme=$(COLORSCHEME) \
		--camera=0,0,0,$(CAM_$*),0 \
		--projection=$(PROJ_$*) \
		-D '$$fn=$(FN)' -D 'explode=$(EXPLODE)' \
		-o $@ $<

# ─── Cleanup ─────────────────────────────────────────────────────────────────

clean:
	rm -rf $(foreach s,$(STEMS),$(RENDER_DIR)/$(s))

clean-all:
	rm -rf $(RENDER_DIR)

# ─── Information ──────────────────────────────────────────────────────────────

list-parts:
	@echo "Discovered $(words $(STEMS)) part(s):"
	@$(foreach s,$(STEMS),echo "  $(s)  ←  $(SRCMAP_$(s))";)
	@echo ""
	@echo "Library files (not rendered):"
	@$(foreach x,$(SCAD_EXCLUDE),echo "  $(x)";)

help:
	@echo "OpenDriveHub Transmitter Enclosure – Build System"
	@echo ""
	@echo "Usage: make [TARGET] [OPTION=value ...]"
	@echo ""
	@echo "Main targets:"
	@echo "  all / renders        Render all parts in all views  ($(words $(ALL_PNGS)) images)"
	@echo "  stl                  Export all parts as STL        ($(words $(ALL_STLS)) files)"
	@echo "  assembly-exploded    Exploded assembly renders"
	@echo "  clean                Remove per-part render directories"
	@echo "  clean-all            Remove entire $(RENDER_DIR)/ directory"
	@echo "  list-parts           Show auto-discovered parts"
	@echo "  help                 Show this help"
	@echo ""
	@echo "Per-part targets:"
	@$(foreach s,$(STEMS),echo "  render-$(s)  /  stl-$(s)";)
	@echo ""
	@echo "Options (override on command line):"
	@echo "  IMG_WIDTH=1920           Render width in pixels"
	@echo "  IMG_HEIGHT=1080          Render height in pixels"
	@echo "  FN=60                    Curve resolution (OpenSCAD \$$fn)"
	@echo "  COLORSCHEME=Cornfield    Color scheme (Cornfield|Metallic|Sunset|...)"
	@echo "  EXPLODE=30               Explosion distance for exploded views"
	@echo "  PNG_MODE=--render         Use --preview to show transparent components"
	@echo "  VIEWS='iso_front ...'    Space-separated list of views to render"
	@echo ""
	@echo "Adding views:"
	@echo "  Define CAM_<name> (rot_x,rot_y,rot_z) and PROJ_<name> (p|o),"
	@echo "  then add <name> to VIEWS."
	@echo ""
	@echo "Examples:"
	@echo "  make renders                                  # All parts, all views"
	@echo "  make -j\$$(nproc) renders                      # Parallel rendering"
	@echo "  make render-assembly                          # Single part, all views"
	@echo "  make render-assembly VIEWS='iso_front top'    # Single part, 2 views"
	@echo "  make assembly-exploded                        # Exploded assembly"
	@echo "  make stl-joystick_zone                        # Single STL export"
	@echo "  make renders IMG_WIDTH=3840 IMG_HEIGHT=2160   # 4K resolution"
