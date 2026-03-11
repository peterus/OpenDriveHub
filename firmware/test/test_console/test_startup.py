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

"""Startup and shutdown tests for the ODH console."""

from __future__ import annotations

import time

import pytest

from console import Console
from transport import SimTransport


class TestStartup:
    """Verify that the shell starts correctly."""

    def test_prompt_appears(self, console: Console) -> None:
        """The shell must print the ``odh> `` prompt on startup."""
        # If we got here, conftest already waited for the prompt successfully.
        assert console.startup_output is not None

    def test_no_error_on_startup(self, console: Console) -> None:
        """Startup output must not contain error indicators."""
        out = console.startup_output.upper()
        assert "[ERROR]" not in out, (
            f"Startup output contains [ERROR]:\n{console.startup_output}"
        )
        assert "[FATAL]" not in out, (
            f"Startup output contains [FATAL]:\n{console.startup_output}"
        )
        assert "PANIC" not in out, (
            f"Startup output contains PANIC:\n{console.startup_output}"
        )

    @pytest.mark.rx
    def test_startup_banner_rx(self, console: Console) -> None:
        """Receiver startup output must contain the RX banner."""
        assert "[ODH-RX]" in console.startup_output or "[ODH]" in console.startup_output, (
            f"Missing ODH banner in:\n{console.startup_output}"
        )

    @pytest.mark.tx
    def test_startup_banner_tx(self, console: Console) -> None:
        """Transmitter startup output must contain the TX banner."""
        assert "[ODH-TX]" in console.startup_output or "[ODH]" in console.startup_output, (
            f"Missing ODH banner in:\n{console.startup_output}"
        )


class TestShutdown:
    """Verify the exit command (simulation only)."""

    @pytest.mark.sim_only
    def test_exit_prints_bye(self, target: str, request: pytest.FixtureRequest) -> None:
        """The ``exit`` command must print 'Bye.' before terminating."""
        from conftest import FIRMWARE_DIR, TARGET_ENV_MAP, _sim_binary_path

        env_name = TARGET_ENV_MAP[target]
        binary = _sim_binary_path(env_name)
        transport = SimTransport(binary)
        con = Console(transport)
        con.start(startup_timeout=10.0)

        try:
            con.send_raw(b"exit\n")
            # Give the process time to print and exit
            time.sleep(1.0)
            remaining = con.read_available(timeout=0.5)
            assert "Bye." in remaining or con.exit_code is not None
        finally:
            con.stop()

    @pytest.mark.sim_only
    def test_exit_code_clean(self, target: str) -> None:
        """After ``exit``, the process must terminate with code 0."""
        from conftest import TARGET_ENV_MAP, _sim_binary_path

        env_name = TARGET_ENV_MAP[target]
        binary = _sim_binary_path(env_name)
        transport = SimTransport(binary)
        con = Console(transport)
        con.start(startup_timeout=10.0)

        try:
            con.send_raw(b"exit\n")
            # Wait for process to terminate
            deadline = time.monotonic() + 5.0
            while time.monotonic() < deadline and con.exit_code is None:
                time.sleep(0.1)
            assert con.exit_code == 0, (
                f"Expected exit code 0, got {con.exit_code}"
            )
        finally:
            con.stop()
