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

"""High-level console helper for interacting with the ODH shell."""

from __future__ import annotations

from typing import Optional

from transport import Transport

PROMPT = "odh> "


class Console:
    """Wraps a :class:`Transport` and provides command-level helpers."""

    def __init__(self, transport: Transport, prompt: str = PROMPT) -> None:
        self._transport = transport
        self._prompt = prompt
        self.startup_output: str = ""

    # -- lifecycle -----------------------------------------------------------

    def start(self, startup_timeout: float = 10.0) -> None:
        """Start the transport and wait for the first prompt.

        The output captured before the prompt is stored in
        :attr:`startup_output`.
        """
        self._transport.start()
        self.startup_output = self.wait_for_prompt(timeout=startup_timeout)

    def stop(self) -> None:
        """Stop the transport."""
        self._transport.stop()

    # -- command helpers -----------------------------------------------------

    def wait_for_prompt(self, timeout: float = 5.0) -> str:
        """Block until the shell prompt appears.

        Returns all output received *before* the prompt.
        """
        raw = self._transport.read_until(self._prompt, timeout=timeout)
        # Strip the trailing prompt from the returned text
        if raw.endswith(self._prompt):
            return raw[: -len(self._prompt)]
        return raw

    def send_command(self, command: str, timeout: float = 5.0) -> str:
        """Send *command*, wait for the next prompt, return the output.

        The returned string does **not** include the typed command echo
        or the trailing prompt.
        """
        self._transport.write((command + "\n").encode("utf-8"))
        raw = self._transport.read_until(self._prompt, timeout=timeout)

        # The shell echoes the command back; strip it from the output.
        if raw.endswith(self._prompt):
            raw = raw[: -len(self._prompt)]

        lines = raw.split("\n")
        # Remove echo line(s) that match the sent command
        output_lines = []
        echo_skipped = False
        for line in lines:
            stripped = line.strip()
            if not echo_skipped and stripped == command.strip():
                echo_skipped = True
                continue
            output_lines.append(line)

        return "\n".join(output_lines).strip()

    def send_raw(self, data: bytes) -> None:
        """Send raw bytes without appending a newline."""
        self._transport.write(data)

    def read_available(self, timeout: float = 0.5) -> str:
        """Read whatever output is available."""
        return self._transport.read_available(timeout=timeout)

    @property
    def exit_code(self) -> Optional[int]:
        """Return the process exit code (sim only), or None."""
        return self._transport.exit_code
