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

"""Receiver-specific console command tests."""

from __future__ import annotations

import pytest

from console import Console

pytestmark = pytest.mark.rx


class TestVehicle:
    """Verify the ``vehicle`` command for getting/setting the vehicle name."""

    def test_vehicle_get(self, console: Console) -> None:
        """``vehicle`` (no args) must return a name string."""
        output = console.send_command("vehicle")
        assert len(output.strip()) > 0, "vehicle returned empty output"

    def test_vehicle_set_get_roundtrip(self, console: Console) -> None:
        """Set vehicle name, then read it back."""
        # Save original
        original = console.send_command("vehicle").strip()

        console.send_command('vehicle "TestCar"')
        output = console.send_command("vehicle")
        assert "TestCar" in output, (
            f"Vehicle name not updated to TestCar:\n{output}"
        )

        # Restore original
        if original:
            console.send_command(f'vehicle "{original}"')


class TestChannelSetRx:
    """Verify ``channel set`` on the receiver."""

    def test_channel_set_valid(self, console: Console) -> None:
        """Setting channel 0 to 1500µs must succeed."""
        output = console.send_command("channel set 0 1500")
        out_lower = output.lower()
        assert "error" not in out_lower and "invalid" not in out_lower, (
            f"Valid channel set failed:\n{output}"
        )

    def test_channel_set_invalid_index(self, console: Console) -> None:
        """Setting an out-of-range channel index must fail."""
        output = console.send_command("channel set 99 1500")
        out_lower = output.lower()
        assert "error" in out_lower or "invalid" in out_lower or "range" in out_lower or "must be" in out_lower, (
            f"Expected error for invalid channel index:\n{output}"
        )

    def test_channel_set_invalid_value(self, console: Console) -> None:
        """Setting an out-of-range PWM value must fail."""
        output = console.send_command("channel set 0 9999")
        out_lower = output.lower()
        assert "error" in out_lower or "invalid" in out_lower or "range" in out_lower or "must be" in out_lower, (
            f"Expected error for invalid PWM value:\n{output}"
        )
