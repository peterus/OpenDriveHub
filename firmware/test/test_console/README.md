# OpenDriveHub Console Tests

End-to-end integration tests for the ODH interactive shell (`odh-shell`).
The framework launches simulation binaries or connects to real hardware via
serial and sends commands through the same console interface a human operator
would use.

---

## Prerequisites

- **Python 3.10+**
- **PlatformIO Core CLI** (for building simulation binaries)

Install the Python dependencies:

```bash
cd firmware/test/test_console
pip install -r requirements.txt
```

The two packages are:

| Package    | Purpose                                |
|------------|----------------------------------------|
| `pytest`   | Test runner and assertion framework    |
| `pyserial` | Serial transport for real hardware     |

---

## Quick Start

```bash
cd firmware/test/test_console

# Run all tests against the receiver simulation (builds automatically)
pytest --target=sim_rx -v

# Run all tests against the transmitter simulation
pytest --target=sim_tx -v
```

On first run, pytest triggers `pio run -e sim_rx` (or `sim_tx`) to build the
binary. Skip with `--no-build` if you already built it:

```bash
pytest --target=sim_rx --no-build -v
```

---

## CLI Options

| Option             | Default   | Description                                     |
|--------------------|-----------|-------------------------------------------------|
| `--target=TARGET`  | `sim_rx`  | Which target to test (see table below)          |
| `--port=PORT`      | —         | Serial port for hardware targets                |
| `--build`          | enabled   | Build sim binary before tests (default)         |
| `--no-build`       | —         | Skip the build step                             |

### Available Targets

| Target       | Transport  | Description                           |
|--------------|------------|---------------------------------------|
| `sim_rx`     | PTY        | Receiver simulation (subprocess)      |
| `sim_tx`     | PTY        | Transmitter simulation (subprocess)   |
| `serial_rx`  | Serial     | Receiver on real hardware via UART    |
| `serial_tx`  | Serial     | Transmitter on real hardware via UART |

### Examples

```bash
# Receiver simulation – verbose output
pytest --target=sim_rx -v

# Transmitter simulation – only startup tests
pytest --target=sim_tx -v test_startup.py

# Real receiver hardware
pytest --target=serial_rx --port=/dev/ttyUSB0 -v

# Run a single test
pytest --target=sim_rx -v test_help.py::TestHelp::test_help_lists_status

# Show print output from tests
pytest --target=sim_rx -v -s
```

---

## Test Structure

```
firmware/test/test_console/
├── conftest.py               # Fixtures, CLI options, markers
├── console.py                # Console helper (send_command, wait_for_prompt)
├── transport.py              # SimTransport (PTY) + SerialTransport (pyserial)
├── requirements.txt          # Python dependencies
├── README.md                 # This file
├── test_startup.py           # Startup banner, prompt, exit/shutdown
├── test_help.py              # help command lists all expected entries
├── test_commands_common.py   # Commands shared by RX + TX
├── test_commands_rx.py       # Receiver-only commands
└── test_commands_tx.py       # Transmitter-only commands
```

### Test Files

| File                      | Tests | Description                                         |
|---------------------------|-------|-----------------------------------------------------|
| `test_startup.py`         | 5     | Prompt appears, no errors, banner, `exit` (sim)     |
| `test_help.py`            | 10    | `help` lists all registered commands                |
| `test_commands_common.py` | 13    | `status`, `config`, `channel`, unknown/empty input  |
| `test_commands_rx.py`     | 5     | `vehicle` get/set, `channel set` valid/invalid      |
| `test_commands_tx.py`     | 8     | `bind`, `trim`, `module`, `disconnect`              |

---

## Architecture

### Transport Layer (`transport.py`)

The abstract `Transport` class defines the I/O interface. Two implementations
exist:

- **`SimTransport`** – Launches the simulation binary under a **PTY**
  (pseudo-terminal). This is necessary because the ODH simulation detects
  piped stdout via `isatty()` and redirects output to `/dev/tty`. Using a PTY
  makes `isatty()` return `true`, so all output goes through the captured
  stream.

- **`SerialTransport`** – Opens a serial connection via `pyserial` at 115200
  baud. A background thread continuously reads incoming bytes into a buffer.

Both transports provide:

```python
transport.start()                            # Open connection / launch process
transport.stop()                             # Close connection / kill process
transport.write(b"help\n")                   # Send raw bytes
transport.read_until("odh> ", timeout=5.0)   # Read until pattern
transport.read_available(timeout=0.5)        # Read whatever is buffered
transport.exit_code                          # Process exit code (sim only)
```

### Console Helper (`console.py`)

The `Console` class wraps a transport and provides command-level methods:

```python
console.start(startup_timeout=10.0)   # Start + wait for first prompt
console.startup_output                 # Everything printed before first prompt
console.send_command("status")         # Send command, wait for prompt, return output
console.send_raw(b"\x1b[A")           # Send raw bytes (e.g. arrow keys)
console.stop()                         # Shut down
```

`send_command()` automatically:
1. Appends `\n` and sends the command
2. Waits for the next `odh> ` prompt
3. Strips the echoed command line from the output
4. Returns only the command's response text

### Fixtures (`conftest.py`)

| Fixture       | Scope    | Description                                  |
|---------------|----------|----------------------------------------------|
| `console`     | module   | Ready `Console` instance (prompt received)   |
| `target`      | session  | Target string (e.g. `"sim_rx"`)              |
| `target_type` | session  | `"rx"` or `"tx"`                             |
| `is_sim`      | session  | `True` for simulation targets                |

The `console` fixture is **module-scoped**: one process per test file. This
keeps tests fast (~5 seconds for all 41 tests) while still isolating test
modules from each other.

### Markers

| Marker       | Effect                                      |
|--------------|---------------------------------------------|
| `@pytest.mark.rx`       | Runs only on receiver targets     |
| `@pytest.mark.tx`       | Runs only on transmitter targets  |
| `@pytest.mark.sim_only` | Runs only on simulation targets   |

Markers are applied automatically – tests with a wrong marker are skipped with
a descriptive reason.

---

## Writing New Tests

### 1. Basic Test

Every test receives the `console` fixture and uses `send_command()`:

```python
from console import Console

class TestMyFeature:
    def test_something(self, console: Console) -> None:
        output = console.send_command("my_command arg1 arg2")
        assert "expected text" in output
```

`send_command()` returns the shell output as a string (without echo and
prompt). Use standard `assert` statements to validate the response.

### 2. Target-Specific Tests

Use markers to restrict tests to receiver or transmitter:

```python
import pytest

pytestmark = pytest.mark.tx   # All tests in this file are TX-only

class TestTransmitterFeature:
    def test_bind_status(self, console: Console) -> None:
        output = console.send_command("bind list")
        assert "error" not in output.lower()
```

Or mark individual tests:

```python
class TestMixed:
    @pytest.mark.rx
    def test_vehicle_name(self, console: Console) -> None:
        output = console.send_command("vehicle")
        assert len(output.strip()) > 0

    @pytest.mark.tx
    def test_trim_value(self, console: Console) -> None:
        output = console.send_command("trim list")
        assert len(output.strip()) > 0
```

### 3. Simulation-Only Tests

For tests that can't run on real hardware (e.g. killing the process):

```python
@pytest.mark.sim_only
def test_exit_terminates(self, console: Console) -> None:
    console.send_raw(b"exit\n")
    time.sleep(1.0)
    assert console.exit_code == 0
```

### 4. Testing Set/Get Roundtrips

A common pattern is to set a value, read it back, then restore the original:

```python
def test_config_roundtrip(self, console: Console) -> None:
    # Save original
    original = console.send_command("config get")

    # Set new value
    console.send_command("config set batt_cell 3")

    # Verify
    output = console.send_command("config get")
    assert "3" in output

    # Restore
    console.send_command("config set batt_cell 0")
```

### 5. Testing Error Responses

Verify that invalid input produces an error message (not a crash):

```python
def test_invalid_argument(self, console: Console) -> None:
    output = console.send_command("channel set 0 99999")
    out_lower = output.lower()
    assert any(kw in out_lower for kw in ("error", "invalid", "must be", "range")), (
        f"Expected error message, got:\n{output}"
    )
```

### 6. Using the `startup_output` Property

To verify what the firmware prints before the shell prompt:

```python
def test_no_crash_on_startup(self, console: Console) -> None:
    assert "PANIC" not in console.startup_output.upper()
    assert "[ERROR]" not in console.startup_output.upper()
```

### 7. Fresh Process per Test (Shutdown Tests)

The `console` fixture is module-scoped (shared across one file). If a test
needs its own process (e.g. to test `exit`), create a transport manually:

```python
from conftest import TARGET_ENV_MAP, _sim_binary_path
from transport import SimTransport

@pytest.mark.sim_only
def test_exit_code(self, target: str) -> None:
    binary = _sim_binary_path(TARGET_ENV_MAP[target])
    transport = SimTransport(binary)
    con = Console(transport)
    con.start(startup_timeout=10.0)
    try:
        con.send_raw(b"exit\n")
        time.sleep(1.0)
        assert con.exit_code == 0
    finally:
        con.stop()
```

---

## Adding a New Test File

1. Create `test_my_feature.py` in `firmware/test/test_console/`
2. Add the GPL-3.0 copyright header (see existing files)
3. Import `Console` and use the `console` fixture
4. Add markers if tests are target-specific
5. Run: `pytest --target=sim_rx -v test_my_feature.py`

### Template

```python
# Copyright (C) 2026 Peter Buchegger
#
# This file is part of OpenDriveHub.
# ... (full GPL-3.0 header)
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""Tests for <describe your feature>."""

from __future__ import annotations

import pytest

from console import Console


class TestMyFeature:
    """Describe what this test class verifies."""

    def test_basic_case(self, console: Console) -> None:
        output = console.send_command("my_command")
        assert "expected" in output.lower()

    @pytest.mark.rx
    def test_rx_specific(self, console: Console) -> None:
        output = console.send_command("vehicle")
        assert len(output.strip()) > 0
```

---

## Timeouts

| Scenario         | Default Timeout | Where to Change          |
|------------------|-----------------|--------------------------|
| Startup          | 10 s            | `conftest.py` → `con.start(startup_timeout=…)` |
| Command response | 5 s             | `console.send_command(cmd, timeout=…)` |
| Read available   | 0.5 s           | `console.read_available(timeout=…)` |

For real hardware, you may need longer timeouts. Override per-command:

```python
output = console.send_command("slow_command", timeout=15.0)
```

---

## Troubleshooting

### `TimeoutError: Timed out waiting for pattern 'odh> '`

- The simulation may not have started. Run `pio run -e sim_rx` manually to
  check for build errors.
- On serial: verify the port and baud rate (`115200`).
- Increase the timeout: `console.send_command("cmd", timeout=15.0)`.

### Tests fail with `FileNotFoundError: Simulation binary not found`

- Build the simulation first: `cd firmware && pio run -e sim_rx`
- Or remove `--no-build` to let pytest build automatically.

### Serial `--port` required error

- Hardware targets need `--port`: `pytest --target=serial_rx --port=/dev/ttyUSB0`

### PTY-related errors on macOS/Windows

- The `SimTransport` uses Linux PTY (`pty.openpty()`). On macOS this should
  work. Windows is not supported for simulation tests (use WSL).
