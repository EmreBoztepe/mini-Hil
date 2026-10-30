"""Verify raw binary USB CDC echo on the Discovery application port."""

import argparse
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="Discovery USB CDC port, for example COM5")
    parser.add_argument("--timeout", type=float, default=3.0, help="Timeout per transfer in seconds")
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")

    import serial

    # This baudrate is CDC line-coding metadata; it does not set the USB bus speed.
    with serial.Serial(args.port, 115200, timeout=0.1, write_timeout=args.timeout) as port:
        port.reset_input_buffer()
        # Includes binary zeros, packet boundaries and multi-packet transfers.
        for length in (1, 63, 64, 65, 127, 128, 257, 1024):
            payload = bytes((i * 37 + length) % 256 for i in range(length))
            deadline = time.monotonic() + args.timeout
            if port.write(payload) != len(payload):
                raise RuntimeError(f"Incomplete write for {length} bytes")
            received = bytearray()
            while len(received) < length and time.monotonic() < deadline:
                received.extend(port.read(length - len(received)))
            if received != payload:
                raise RuntimeError(f"Echo mismatch: sent {length}, received {len(received)} bytes")
            print(f"PASS: {length} bytes")
    print("USB CDC binary echo passed.")


if __name__ == "__main__":
    main()
