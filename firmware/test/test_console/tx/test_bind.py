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

"""Transmitter-specific tests for ``bind`` and ``disconnect`` commands."""

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
        assert "error" not in output.lower(), (
            f"bind list returned error:\n{output}"
        )


class TestBindHelp:
    """Verify ``bind help`` shows sub-commands."""

    def test_bind_help(self, console: Console) -> None:
        output = console.send_command("bind help")
        out_lower = output.lower()
        assert "scan" in out_lower, (
            f"bind help missing 'scan':\n{output}"
        )
        assert "list" in out_lower, (
            f"bind help missing 'list':\n{output}"
        )
        assert "connect" in out_lower, (
            f"bind help missing 'connect':\n{output}"
        )


class TestDisconnect:
    """Verify ``disconnect`` when not connected."""

    def test_disconnect_when_idle(self, console: Console) -> None:
        """``disconnect`` when not connected must not crash."""
        output = console.send_command("disconnect")
        assert "error" not in output.lower() or "not connected" in output.lower(), (
            f"disconnect produced unexpected error:\n{output}"
        )
