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

"""Transmitter-specific config roundtrip tests."""

from __future__ import annotations

import pytest

from console import Console

pytestmark = pytest.mark.tx


class TestConfigRoundtripTx:
    """Verify ``config set`` followed by ``config get`` persists values."""

    def test_config_set_get_tx_cells(self, console: Console) -> None:
        """TX: set tx_cells, read it back."""
        console.send_command("config set tx_cells 3")
        output = console.send_command("config get")
        assert "3" in output, (
            f"tx_cells not set to 3 in config get:\n{output}"
        )
        console.send_command("config set tx_cells 0")
