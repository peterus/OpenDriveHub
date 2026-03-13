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

"""Basic shell interaction tests (unknown command, empty line, sequential)."""

from __future__ import annotations

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
        assert "error" not in output.lower(), (
            f"Empty line produced an error:\n{output}"
        )
        assert "unknown" not in output.lower(), (
            f"Empty line triggered 'unknown command':\n{output}"
        )


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
