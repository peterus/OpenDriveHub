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

"""Tests for the ``config`` command (shared by RX and TX)."""

from __future__ import annotations

import pytest

from console import Console


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


class TestReboot:
    """Verify ``reboot`` in simulation."""

    @pytest.mark.sim_only
    def test_reboot_not_available_in_sim(self, console: Console) -> None:
        output = console.send_command("reboot")
        out_lower = output.lower()
        assert "not available" in out_lower or "simulation" in out_lower, (
            f"Expected 'not available in simulation':\n{output}"
        )
