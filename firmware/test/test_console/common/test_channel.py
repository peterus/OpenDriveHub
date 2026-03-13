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

"""Tests for the ``channel`` command (shared by RX and TX)."""

from __future__ import annotations

from console import Console


class TestChannelGet:
    """Verify ``channel get`` shows channel information."""

    def test_channel_get_returns_output(self, console: Console) -> None:
        output = console.send_command("channel get")
        assert len(output.strip()) > 0, "channel get returned empty output"


class TestChannelGetNotList:
    """Verify old ``channel list`` syntax is rejected."""

    def test_channel_list_rejected(self, console: Console) -> None:
        output = console.send_command("channel list")
        out_lower = output.lower()
        assert "unknown" in out_lower or "usage" in out_lower or "error" in out_lower, (
            f"Expected error for old 'channel list' syntax:\n{output}"
        )
