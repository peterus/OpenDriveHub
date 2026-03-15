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

"""Tests for the ``status`` command (shared by RX and TX)."""

from __future__ import annotations

from console import Console


class TestStatus:
    """Verify the ``status`` command produces valid output."""

    def test_status_returns_output(self, console: Console) -> None:
        output = console.send_command("status")
        assert len(output.strip()) > 0, "status returned empty output"

    def test_status_contains_link_info(self, console: Console) -> None:
        """``status`` must mention some kind of link/connection state."""
        output = console.send_command("status").lower()
        has_link_info = any(
            kw in output
            for kw in ("link", "connected", "disconnected", "scanning",
                        "announcing", "idle", "state")
        )
        assert has_link_info, (
            f"status output missing link info:\n{output}"
        )

    def test_status_contains_battery(self, console: Console) -> None:
        """``status`` must mention battery information."""
        output = console.send_command("status").lower()
        has_battery = any(
            kw in output for kw in ("battery", "batt", "voltage", "cell")
        )
        assert has_battery, (
            f"status output missing battery info:\n{output}"
        )
