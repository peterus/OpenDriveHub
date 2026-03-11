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

"""Transmitter-specific console command tests."""

from __future__ import annotations

import pytest

from console import Console

pytestmark = pytest.mark.tx


class TestBind:
    """Verify ``bind`` subcommands."""

    def test_bind_scan(self, console: Console) -> None:
        """``bind scan`` must not crash."""
        output = console.send_command("bind scan")
        out_lower = output.lower()
        assert "error" not in out_lower and "unknown" not in out_lower, (
            f"bind scan failed:\n{output}"
        )

    def test_bind_list(self, console: Console) -> None:
        """``bind list`` must return output (may be empty list)."""
        output = console.send_command("bind list")
        # Might say "no vehicles" or list entries – both are fine
        assert "error" not in output.lower(), (
            f"bind list returned error:\n{output}"
        )


class TestDisconnect:
    """Verify ``disconnect`` when not connected."""

    def test_disconnect_when_idle(self, console: Console) -> None:
        """``disconnect`` when not connected must not crash."""
        output = console.send_command("disconnect")
        # May say "not connected" or similar – that's fine
        assert "error" not in output.lower() or "not connected" in output.lower(), (
            f"disconnect produced unexpected error:\n{output}"
        )


class TestTrim:
    """Verify ``trim`` subcommands."""

    def test_trim_list(self, console: Console) -> None:
        """``trim list`` must return formatted output."""
        output = console.send_command("trim list")
        assert len(output.strip()) > 0, "trim list returned empty output"

    def test_trim_set_get_roundtrip(self, console: Console) -> None:
        """Set trim, verify it appears in ``trim list``."""
        console.send_command("trim set 0 50")
        output = console.send_command("trim list")
        assert "50" in output, (
            f"trim value 50 not found in trim list:\n{output}"
        )
        # Reset trim
        console.send_command("trim set 0 0")

    def test_trim_set_invalid_range(self, console: Console) -> None:
        """Setting trim outside -100..+100 must fail."""
        output = console.send_command("trim set 0 200")
        out_lower = output.lower()
        assert "error" in out_lower or "invalid" in out_lower or "range" in out_lower or "must be" in out_lower, (
            f"Expected error for out-of-range trim:\n{output}"
        )


class TestModule:
    """Verify ``module`` command."""

    def test_module_list(self, console: Console) -> None:
        """``module`` must return output (may report no modules in sim)."""
        output = console.send_command("module")
        assert "error" not in output.lower(), (
            f"module returned error:\n{output}"
        )
