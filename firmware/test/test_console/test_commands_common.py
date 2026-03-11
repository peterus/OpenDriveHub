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


class TestChannelGet:
    """Verify ``channel get`` shows channel information."""

    def test_channel_get_returns_output(self, console: Console) -> None:
        output = console.send_command("channel get")
        assert len(output.strip()) > 0, "channel get returned empty output"


class TestMultipleCommands:
    """Verify that running several commands in sequence works."""

    def test_sequential_commands(self, console: Console) -> None:
        """Run 5 different commands and ensure all return valid output."""
        commands = ["help", "status", "config get", "channel get", "help"]
        for cmd in commands:
            output = console.send_command(cmd)
            assert len(output.strip()) > 0, (
                f"Command {cmd!r} returned empty output"
            )
            assert "error" not in output.lower() or cmd == "config set", (
                f"Command {cmd!r} produced error:\n{output}"
            )


class TestConfigHelp:
    """Verify ``config help`` lists available keys with value ranges."""

    def test_config_help_returns_output(self, console: Console) -> None:
        output = console.send_command("config help")
        assert len(output.strip()) > 0, "config help returned empty output"

    def test_config_help_lists_radio_ch(self, console: Console) -> None:
        output = console.send_command("config help")
        assert "radio_ch" in output, (
            f"config help missing radio_ch:\n{output}"
        )

    def test_config_help_lists_model(self, console: Console) -> None:
        output = console.send_command("config help")
        assert "model" in output.lower(), (
            f"config help missing model:\n{output}"
        )

    def test_config_help_shows_valid_channels(self, console: Console) -> None:
        """``config help`` must mention valid WiFi channels."""
        output = console.send_command("config help")
        assert "1" in output and "6" in output and "11" in output, (
            f"config help missing valid channel values:\n{output}"
        )


class TestConfigReset:
    """Verify ``config reset`` works without error."""

    def test_config_reset(self, console: Console) -> None:
        output = console.send_command("config reset")
        out_lower = output.lower()
        assert "error" not in out_lower and "unknown" not in out_lower, (
            f"config reset failed:\n{output}"
        )


class TestConfigSetRadioCh:
    """Verify ``config set radio_ch`` validates channels."""

    def test_config_set_radio_ch_invalid(self, console: Console) -> None:
        """Setting radio_ch to 3 (not 1/6/11) must fail."""
        output = console.send_command("config set radio_ch 3")
        out_lower = output.lower()
        assert "invalid" in out_lower or "error" in out_lower, (
            f"Expected error for invalid channel 3:\n{output}"
        )

    def test_config_set_radio_ch_valid(self, console: Console) -> None:
        """Setting radio_ch to 6 must succeed."""
        output = console.send_command("config set radio_ch 6")
        out_lower = output.lower()
        assert "error" not in out_lower and "invalid" not in out_lower, (
            f"Valid channel 6 was rejected:\n{output}"
        )
        result = console.send_command("config get")
        assert "6" in result, (
            f"radio_ch not set to 6:\n{result}"
        )
        # Reset to default
        console.send_command("config set radio_ch 1")


class TestConfigSetModel:
    """Verify ``config set model`` works."""

    def test_config_set_model_valid(self, console: Console) -> None:
        """Setting model to a valid type must succeed."""
        output = console.send_command("config set model Generic")
        out_lower = output.lower()
        assert "error" not in out_lower and "unknown" not in out_lower, (
            f"Valid model was rejected:\n{output}"
        )

    def test_config_set_model_invalid(self, console: Console) -> None:
        """Setting an invalid model must fail and list available models."""
        output = console.send_command("config set model spaceship")
        out_lower = output.lower()
        assert "unknown" in out_lower or "invalid" in out_lower or "error" in out_lower, (
            f"Expected error for invalid model:\n{output}"
        )
        assert "generic" in out_lower, (
            f"Expected available models to be listed:\n{output}"
        )


class TestChannelGetNotList:
    """Verify old ``channel list`` syntax is rejected."""

    def test_channel_list_rejected(self, console: Console) -> None:
        output = console.send_command("channel list")
        out_lower = output.lower()
        assert "unknown" in out_lower or "usage" in out_lower or "error" in out_lower, (
            f"Expected error for old 'channel list' syntax:\n{output}"
        )


class TestReboot:
    """Verify ``reboot`` in simulation."""

    @pytest.mark.sim_only
    def test_reboot_not_available_in_sim(self, console: Console) -> None:
        output = console.send_command("reboot")
        out_lower = output.lower()
        assert "not available" in out_lower or "simulation" in out_lower, (
            f"Expected 'not available in simulation':\n{output}"
        )
