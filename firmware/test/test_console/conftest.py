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

"""Pytest configuration and fixtures for ODH console tests."""

from __future__ import annotations

import os
import subprocess
import sys

import pytest

from console import Console
from transport import SerialTransport, SimTransport

FIRMWARE_DIR = os.path.normpath(
    os.path.join(os.path.dirname(__file__), os.pardir, os.pardir)
)

TARGET_ENV_MAP = {
    "sim_rx": "sim_rx",
    "sim_tx": "sim_tx",
    "serial_rx": None,
    "serial_tx": None,
}

TARGET_ROLE = {
    "sim_rx": "rx",
    "sim_tx": "tx",
    "serial_rx": "rx",
    "serial_tx": "tx",
}


# -- CLI options -------------------------------------------------------------


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--target",
        choices=list(TARGET_ENV_MAP.keys()),
        default="sim_rx",
        help="Target to test against (default: sim_rx)",
    )
    parser.addoption(
        "--port",
        default=None,
        help="Serial port for hardware targets (e.g. /dev/ttyUSB0)",
    )
    parser.addoption(
        "--build",
        action="store_true",
        default=True,
        dest="build",
        help="Build the simulation binary before running tests (default)",
    )
    parser.addoption(
        "--no-build",
        action="store_false",
        dest="build",
        help="Skip building the simulation binary",
    )


# -- markers -----------------------------------------------------------------


def pytest_configure(config: pytest.Config) -> None:
    config.addinivalue_line("markers", "rx: test only runs on receiver targets")
    config.addinivalue_line(
        "markers", "tx: test only runs on transmitter targets"
    )
    config.addinivalue_line(
        "markers", "sim_only: test only runs on simulation targets"
    )


def pytest_collection_modifyitems(
    config: pytest.Config, items: list[pytest.Item]
) -> None:
    target = config.getoption("--target")
    role = TARGET_ROLE[target]
    is_sim = target.startswith("sim_")

    for item in items:
        if "rx" in item.keywords and role != "rx":
            item.add_marker(
                pytest.mark.skip(reason=f"RX-only test, target is {target}")
            )
        if "tx" in item.keywords and role != "tx":
            item.add_marker(
                pytest.mark.skip(reason=f"TX-only test, target is {target}")
            )
        if "sim_only" in item.keywords and not is_sim:
            item.add_marker(
                pytest.mark.skip(
                    reason=f"Sim-only test, target is {target}"
                )
            )


# -- fixtures ----------------------------------------------------------------


@pytest.fixture(scope="session")
def target(request: pytest.FixtureRequest) -> str:
    return request.config.getoption("--target")


@pytest.fixture(scope="session")
def target_type(target: str) -> str:
    """Return ``'rx'`` or ``'tx'``."""
    return TARGET_ROLE[target]


@pytest.fixture(scope="session")
def is_sim(target: str) -> bool:
    return target.startswith("sim_")


def _build_sim(env_name: str) -> None:
    """Run ``pio run -e <env>`` to build the simulation binary."""
    print(f"\n>>> Building {env_name} …")
    result = subprocess.run(
        ["pio", "run", "-e", env_name, "--silent"],
        cwd=FIRMWARE_DIR,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr, file=sys.stderr)
        pytest.fail(f"Build of {env_name} failed (exit {result.returncode})")


def _sim_binary_path(env_name: str) -> str:
    return os.path.join(FIRMWARE_DIR, ".pio", "build", env_name, "program")


@pytest.fixture(scope="module")
def console(request: pytest.FixtureRequest, target: str) -> Console:
    """Yield a ready :class:`Console` with the first prompt already received."""

    is_sim_target = target.startswith("sim_")

    if is_sim_target:
        env_name = TARGET_ENV_MAP[target]
        if request.config.getoption("build"):
            _build_sim(env_name)

        binary = _sim_binary_path(env_name)
        transport = SimTransport(binary)
    else:
        port = request.config.getoption("--port")
        if port is None:
            pytest.fail(
                f"--port is required for serial target {target!r}"
            )
        transport = SerialTransport(port)

    con = Console(transport)
    con.start(startup_timeout=10.0)
    yield con
    con.stop()
