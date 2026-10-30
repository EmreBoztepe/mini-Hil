"""Compile and run Platform tests with a native C compiler and USB test doubles."""

import os
from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
PLATFORM = ROOT / "firmware" / "stm32f407_discovery" / "Platform"


def main():
    with tempfile.TemporaryDirectory(prefix="mini-hil-host-") as directory:
        executable = Path(directory) / ("test_usb.exe" if os.name == "nt" else "test_usb")
        subprocess.run(
            [
                os.environ.get("CC", "gcc"), "-std=c11", "-Wall", "-Wextra", "-Werror",
                "-I", str(HERE / "mocks"), "-I", str(PLATFORM),
                str(PLATFORM / "hil_usb_cdc.c"), str(HERE / "test_hil_usb_cdc.c"),
                "-o", str(executable),
            ],
            check=True,
        )
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
