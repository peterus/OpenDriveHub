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

"""Transport layer for console tests.

Provides an abstract Transport interface with two implementations:
- SimTransport: launches a simulation binary as a subprocess
- SerialTransport: connects to real hardware via pyserial
"""

import abc
import os
import pty
import subprocess
import threading
import time
from typing import Optional

import serial


class Transport(abc.ABC):
    """Abstract base for console I/O transports."""

    @abc.abstractmethod
    def start(self) -> None:
        """Open the connection / launch the process."""

    @abc.abstractmethod
    def stop(self) -> None:
        """Close the connection / kill the process."""

    @abc.abstractmethod
    def write(self, data: bytes) -> None:
        """Send raw bytes to the console."""

    @abc.abstractmethod
    def read_until(self, pattern: str, timeout: float = 5.0) -> str:
        """Read output until *pattern* is found or *timeout* expires.

        Returns all captured output (including the pattern).
        Raises ``TimeoutError`` with partial output on timeout.
        """

    @abc.abstractmethod
    def read_available(self, timeout: float = 0.5) -> str:
        """Read whatever output is available within *timeout* seconds."""

    @property
    @abc.abstractmethod
    def exit_code(self) -> Optional[int]:
        """Return the process exit code, or ``None`` if still running / N/A."""


class SimTransport(Transport):
    """Launch a PlatformIO simulation binary via a PTY.

    The simulation binary uses ``isatty()`` to decide whether to write to
    stdout or ``/dev/tty``.  By running it under a pseudo-terminal, stdout
    is a TTY and all output goes through the PTY – which we can capture.
    """

    def __init__(self, binary_path: str) -> None:
        self._binary_path = binary_path
        self._proc: Optional[subprocess.Popen] = None
        self._master_fd: Optional[int] = None
        self._output_buf = ""
        self._buf_lock = threading.Lock()
        self._reader_thread: Optional[threading.Thread] = None
        self._running = False

    # -- lifecycle -----------------------------------------------------------

    def start(self) -> None:
        if not os.path.isfile(self._binary_path):
            raise FileNotFoundError(
                f"Simulation binary not found: {self._binary_path}"
            )

        # Create a PTY so the child sees a real terminal (isatty → true).
        master_fd, slave_fd = pty.openpty()
        self._master_fd = master_fd

        self._proc = subprocess.Popen(
            [self._binary_path],
            stdin=slave_fd,
            stdout=slave_fd,
            stderr=slave_fd,
            close_fds=True,
        )
        os.close(slave_fd)

        self._running = True
        self._reader_thread = threading.Thread(
            target=self._reader_loop, daemon=True
        )
        self._reader_thread.start()

    def stop(self) -> None:
        self._running = False
        if self._proc is not None:
            try:
                self._proc.terminate()
                self._proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self._proc.kill()
                self._proc.wait(timeout=2)
        if self._master_fd is not None:
            try:
                os.close(self._master_fd)
            except OSError:
                pass
            self._master_fd = None

    # -- I/O -----------------------------------------------------------------

    def write(self, data: bytes) -> None:
        assert self._master_fd is not None
        os.write(self._master_fd, data)

    def read_until(self, pattern: str, timeout: float = 5.0) -> str:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            with self._buf_lock:
                idx = self._output_buf.find(pattern)
                if idx != -1:
                    end = idx + len(pattern)
                    result = self._output_buf[:end]
                    self._output_buf = self._output_buf[end:]
                    return result
            if self._proc is not None and self._proc.poll() is not None:
                with self._buf_lock:
                    remaining = self._output_buf
                    self._output_buf = ""
                raise TimeoutError(
                    f"Process exited (code {self._proc.returncode}) before "
                    f"pattern {pattern!r} was found. Output so far:\n{remaining}"
                )
            time.sleep(0.02)

        with self._buf_lock:
            partial = self._output_buf
            self._output_buf = ""
        raise TimeoutError(
            f"Timed out ({timeout}s) waiting for pattern {pattern!r}. "
            f"Output so far:\n{partial}"
        )

    def read_available(self, timeout: float = 0.5) -> str:
        time.sleep(timeout)
        with self._buf_lock:
            result = self._output_buf
            self._output_buf = ""
        return result

    @property
    def exit_code(self) -> Optional[int]:
        if self._proc is None:
            return None
        return self._proc.poll()

    # -- internals -----------------------------------------------------------

    def _reader_loop(self) -> None:
        assert self._master_fd is not None
        try:
            while self._running:
                try:
                    data = os.read(self._master_fd, 4096)
                except OSError:
                    break
                if not data:
                    break
                text = data.decode("utf-8", errors="replace")
                with self._buf_lock:
                    self._output_buf += text
        except (OSError, ValueError):
            pass


class SerialTransport(Transport):
    """Communicate with real hardware over a serial (UART) connection."""

    DEFAULT_BAUD = 115200

    def __init__(self, port: str, baud: int = DEFAULT_BAUD) -> None:
        self._port = port
        self._baud = baud
        self._ser: Optional[serial.Serial] = None
        self._output_buf = ""
        self._buf_lock = threading.Lock()
        self._reader_thread: Optional[threading.Thread] = None
        self._running = False

    # -- lifecycle -----------------------------------------------------------

    def start(self) -> None:
        self._ser = serial.Serial(
            self._port,
            self._baud,
            timeout=0.1,
            write_timeout=2,
        )
        self._ser.reset_input_buffer()
        self._running = True
        self._reader_thread = threading.Thread(
            target=self._reader_loop, daemon=True
        )
        self._reader_thread.start()

    def stop(self) -> None:
        self._running = False
        if self._ser is not None:
            try:
                self._ser.close()
            except OSError:
                pass

    # -- I/O -----------------------------------------------------------------

    def write(self, data: bytes) -> None:
        assert self._ser is not None
        self._ser.write(data)
        self._ser.flush()

    def read_until(self, pattern: str, timeout: float = 5.0) -> str:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            with self._buf_lock:
                idx = self._output_buf.find(pattern)
                if idx != -1:
                    end = idx + len(pattern)
                    result = self._output_buf[:end]
                    self._output_buf = self._output_buf[end:]
                    return result
            time.sleep(0.02)

        with self._buf_lock:
            partial = self._output_buf
            self._output_buf = ""
        raise TimeoutError(
            f"Timed out ({timeout}s) waiting for pattern {pattern!r}. "
            f"Output so far:\n{partial}"
        )

    def read_available(self, timeout: float = 0.5) -> str:
        time.sleep(timeout)
        with self._buf_lock:
            result = self._output_buf
            self._output_buf = ""
        return result

    @property
    def exit_code(self) -> Optional[int]:
        return None

    # -- internals -----------------------------------------------------------

    def _reader_loop(self) -> None:
        assert self._ser is not None
        try:
            while self._running:
                data = self._ser.read(256)
                if data:
                    text = data.decode("utf-8", errors="replace")
                    with self._buf_lock:
                        self._output_buf += text
        except (OSError, serial.SerialException):
            pass
