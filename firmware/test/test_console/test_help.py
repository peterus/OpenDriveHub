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

"""Tests for the ``help`` command."""

from __future__ import annotations

import pytest

from console import Console


class TestHelp:
    """Verify that ``help`` lists all expected commands."""

    def test_help_returns_output(self, console: Console) -> None:
        """``help`` must produce non-empty output."""
        output = console.send_command("help")
        assert len(output.strip()) > 0, "help returned empty output"

    def test_help_lists_help(self, console: Console) -> None:
        output = console.send_command("help")
        assert "help" in output.lower()

    def test_help_lists_status(self, console: Console) -> None:
        output = console.send_command("help")
        assert "status" in output.lower()

    def test_help_lists_config(self, console: Console) -> None:
        output = console.send_command("help")
        assert "config" in output.lower()

    def test_help_lists_channel(self, console: Console) -> None:
        output = console.send_command("help")
        assert "channel" in output.lower()

    @pytest.mark.rx
    def test_help_lists_vehicle(self, console: Console) -> None:
        """RX ``help`` must list the ``vehicle`` command."""
        output = console.send_command("help")
        assert "vehicle" in output.lower()

    @pytest.mark.rx
    def test_help_lists_mapping(self, console: Console) -> None:
        """RX ``help`` must list the ``mapping`` command."""
        output = console.send_command("help")
        assert "mapping" in output.lower()

    @pytest.mark.rx
    def test_help_lists_failsafe(self, console: Console) -> None:
        """RX ``help`` must list the ``failsafe`` command."""
        output = console.send_command("help")
        assert "failsafe" in output.lower()

    @pytest.mark.rx
    def test_help_lists_reboot_rx(self, console: Console) -> None:
        """RX ``help`` must list the ``reboot`` command."""
        output = console.send_command("help")
        assert "reboot" in output.lower()

    @pytest.mark.tx
    def test_help_lists_bind(self, console: Console) -> None:
        """TX ``help`` must list the ``bind`` command."""
        output = console.send_command("help")
        assert "bind" in output.lower()

    @pytest.mark.tx
    def test_help_lists_trim(self, console: Console) -> None:
        """TX ``help`` must list the ``trim`` command."""
        output = console.send_command("help")
        assert "trim" in output.lower()

    @pytest.mark.tx
    def test_help_lists_module(self, console: Console) -> None:
        """TX ``help`` must list the ``module`` command."""
        output = console.send_command("help")
        assert "module" in output.lower()

    @pytest.mark.tx
    def test_help_lists_disconnect(self, console: Console) -> None:
        """TX ``help`` must list the ``disconnect`` command."""
        output = console.send_command("help")
        assert "disconnect" in output.lower()

    @pytest.mark.tx
    def test_help_lists_input(self, console: Console) -> None:
        """TX ``help`` must list the ``input`` command."""
        output = console.send_command("help")
        assert "input" in output.lower()

    @pytest.mark.tx
    def test_help_lists_rescan(self, console: Console) -> None:
        """TX ``help`` must list the ``rescan`` command."""
        output = console.send_command("help")
        assert "rescan" in output.lower()

    @pytest.mark.tx
    def test_help_lists_reboot_tx(self, console: Console) -> None:
        """TX ``help`` must list the ``reboot`` command."""
        output = console.send_command("help")
        assert "reboot" in output.lower()
