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

"""Tests for commands shared by both receiver and transmitter."""

from __future__ import annotations

import pytest

from console import Console


class TestUnknownCommand:
    """Verify behaviour when the user enters an unrecognised command."""

    def test_unknown_command_error_message(self, console: Console) -> None:
        output = console.send_command("foobar_nonexistent")
        assert "unknown command" in output.lower(), (
            f"Expected 'Unknown command' error, got:\n{output}"
        )

    def test_unknown_command_suggests_help(self, console: Console) -> None:
        output = console.send_command("notacommand")
        assert "help" in output.lower(), (
            f"Expected suggestion to type 'help', got:\n{output}"
        )


class TestEmptyLine:
    """Verify that pressing Enter without a command does nothing harmful."""

    def test_empty_line_no_error(self, console: Console) -> None:
        """An empty line should just produce a new prompt (no error)."""
        output = console.send_command("")
        # Empty line should produce no output or just whitespace
        assert "error" not in output.lower(), (
            f"Empty line produced an error:\n{output}"
        )
        assert "unknown" not in output.lower(), (
            f"Empty line triggered 'unknown command':\n{output}"
        )


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


class TestConfigGet:
    """Verify ``config get`` returns key-value data."""

    def test_config_get_returns_output(self, console: Console) -> None:
        output = console.send_command("config get")
        assert len(output.strip()) > 0, "config get returned empty output"

    def test_config_get_contains_radio_ch(self, console: Console) -> None:
        output = console.send_command("config get")
        assert "radio_ch" in output, (
            f"config get missing radio_ch:\n{output}"
        )

    def test_config_set_invalid_key(self, console: Console) -> None:
        """Setting an invalid key must produce an error."""
        output = console.send_command("config set totally_invalid_key 42")
        out_lower = output.lower()
        assert "error" in out_lower or "unknown" in out_lower or "invalid" in out_lower, (
            f"Expected error for invalid key, got:\n{output}"
        )


class TestConfigRoundtrip:
    """Verify ``config set`` followed by ``config get`` persists values."""

    @pytest.mark.rx
    def test_config_set_get_batt_cell_rx(self, console: Console) -> None:
        """RX: set batt_cell, read it back."""
        console.send_command("config set batt_cell 3")
        output = console.send_command("config get")
        assert "3" in output, (
            f"batt_cell not set to 3 in config get:\n{output}"
        )
        # Reset to auto
        console.send_command("config set batt_cell 0")

    @pytest.mark.tx
    def test_config_set_get_tx_cells(self, console: Console) -> None:
        """TX: set tx_cells, read it back."""
        console.send_command("config set tx_cells 3")
        output = console.send_command("config get")
        assert "3" in output, (
            f"tx_cells not set to 3 in config get:\n{output}"
        )
        # Reset to auto
        console.send_command("config set tx_cells 0")


class TestChannelList:
    """Verify ``channel list`` shows channel information."""

    def test_channel_list_returns_output(self, console: Console) -> None:
        output = console.send_command("channel list")
        assert len(output.strip()) > 0, "channel list returned empty output"


class TestMultipleCommands:
    """Verify that running several commands in sequence works."""

    def test_sequential_commands(self, console: Console) -> None:
        """Run 5 different commands and ensure all return valid output."""
        commands = ["help", "status", "config get", "channel list", "help"]
        for cmd in commands:
            output = console.send_command(cmd)
            assert len(output.strip()) > 0, (
                f"Command {cmd!r} returned empty output"
            )
            assert "error" not in output.lower() or cmd == "config set", (
                f"Command {cmd!r} produced error:\n{output}"
            )
