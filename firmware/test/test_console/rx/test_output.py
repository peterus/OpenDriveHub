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

"""Receiver-specific tests for ``mapping`` and ``failsafe`` commands."""

from __future__ import annotations

import pytest

from console import Console

pytestmark = pytest.mark.rx


class TestMapping:
    """Verify the ``mapping`` command for function-to-channel mapping."""

    def test_mapping_get(self, console: Console) -> None:
        """``mapping get`` must show channel-to-function assignments."""
        output = console.send_command("mapping get")
        assert len(output.strip()) > 0, "mapping get returned empty output"
        assert "ch" in output.lower(), (
            f"mapping get missing channel info:\n{output}"
        )

    def test_mapping_help(self, console: Console) -> None:
        """``mapping help`` must list available function names."""
        output = console.send_command("mapping help")
        out_lower = output.lower()
        assert "drive" in out_lower, (
            f"mapping help missing 'Drive' function:\n{output}"
        )
        assert "steering" in out_lower, (
            f"mapping help missing 'Steering' function:\n{output}"
        )

    def test_mapping_reset(self, console: Console) -> None:
        """``mapping reset`` must succeed without error."""
        output = console.send_command("mapping reset")
        out_lower = output.lower()
        assert "error" not in out_lower and "unknown" not in out_lower, (
            f"mapping reset failed:\n{output}"
        )

    def test_mapping_no_subcommand(self, console: Console) -> None:
        """``mapping`` without subcommand must show usage."""
        output = console.send_command("mapping")
        out_lower = output.lower()
        assert "usage" in out_lower or "get" in out_lower, (
            f"Expected usage hint:\n{output}"
        )

    def test_mapping_reset_roundtrip(self, console: Console) -> None:
        """Set model to Excavator, reset mapping, verify defaults."""
        console.send_command("config set model Excavator")
        console.send_command("mapping reset")
        output = console.send_command("mapping get")
        out_lower = output.lower()
        assert "boom" in out_lower or "bucket" in out_lower or "track" in out_lower, (
            f"Excavator mapping not applied:\n{output}"
        )
        console.send_command("config set model Generic")
        console.send_command("mapping reset")


class TestFailsafe:
    """Verify the ``failsafe`` command."""

    def test_failsafe_get(self, console: Console) -> None:
        """``failsafe get`` must show failsafe values per channel."""
        output = console.send_command("failsafe get")
        assert len(output.strip()) > 0, "failsafe get returned empty output"
        assert "ch" in output.lower(), (
            f"failsafe get missing channel info:\n{output}"
        )

    def test_failsafe_set(self, console: Console) -> None:
        """Set failsafe value and verify it sticks."""
        console.send_command("failsafe set 0 1200")
        output = console.send_command("failsafe get")
        assert "1200" in output, (
            f"Failsafe value 1200 not found:\n{output}"
        )
        console.send_command("failsafe set 0 1500")

    def test_failsafe_set_invalid_channel(self, console: Console) -> None:
        """Setting failsafe on invalid channel must fail."""
        output = console.send_command("failsafe set 99 1500")
        out_lower = output.lower()
        assert "error" in out_lower or "range" in out_lower or "invalid" in out_lower, (
            f"Expected error for invalid channel:\n{output}"
        )

    def test_failsafe_set_invalid_value(self, console: Console) -> None:
        """Setting failsafe to out-of-range value must fail."""
        output = console.send_command("failsafe set 0 500")
        out_lower = output.lower()
        assert "error" in out_lower or "range" in out_lower or "invalid" in out_lower, (
            f"Expected error for invalid value:\n{output}"
        )

    def test_failsafe_no_subcommand(self, console: Console) -> None:
        """``failsafe`` without subcommand must show usage."""
        output = console.send_command("failsafe")
        out_lower = output.lower()
        assert "usage" in out_lower or "get" in out_lower, (
            f"Expected usage hint:\n{output}"
        )
