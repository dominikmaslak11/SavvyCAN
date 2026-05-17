#!/usr/bin/env python3
"""
SavvyCAN PCAN Frame Sender
Sends extended CAN frame 0x1421003F every 10 seconds at 250 kbps.
"""

import can
import time
import signal
import sys

# ── Configuration ────────────────────────────────────────────────────
BITRATE = 250000          # 250 kbps
FRAME_ID = 0x1421003F     # Extended CAN ID
DATA = [0x50, 0x01, 0x00, 0x00, 0x00, 0x32, 0x19, 0xB1]
INTERVAL = 10.0           # seconds
INTERFACE = "pcan"        # PEAK PCAN USB
CHANNEL = "PCAN_USBBUS1"  # First PCAN USB device

running = True

def signal_handler(sig, frame):
    global running
    print("\nStopping...")
    running = False

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def main():
    print(f"Opening {INTERFACE} bus on {CHANNEL} at {BITRATE} bps...")

    try:
        bus = can.interface.Bus(
            interface=INTERFACE,
            channel=CHANNEL,
            bitrate=BITRATE
        )
        print(f"Connected: {bus}")
    except Exception as e:
        print(f"Failed to open bus: {e}")
        print("Trying with channel=0...")
        try:
            bus = can.interface.Bus(
                interface=INTERFACE,
                channel="0",
                bitrate=BITRATE
            )
            print(f"Connected: {bus}")
        except Exception as e2:
            print(f"Also failed: {e2}")
            print("\nAvailable PCAN channels:")
            try:
                for ch in range(4):
                    try:
                        test_bus = can.interface.Bus(interface="pcan", channel=str(ch), bitrate=BITRATE)
                        print(f"  Channel {ch}: OK")
                        test_bus.shutdown()
                    except:
                        print(f"  Channel {ch}: not available")
            except:
                pass
            return 1

    msg = can.Message(
        arbitration_id=FRAME_ID,
        is_extended_id=True,
        data=DATA
    )

    count = 0
    print(f"\nSending frame every {INTERVAL}s:")
    print(f"  ID:  0x{FRAME_ID:08X} (extended)")
    print(f"  DLC: {len(DATA)}")
    print(f"  Data: {' '.join(f'{b:02X}' for b in DATA)}")
    print(f"  Press Ctrl+C to stop\n")

    try:
        while running:
            bus.send(msg)
            count += 1
            print(f"[{count}] Sent 0x{FRAME_ID:08X}  {' '.join(f'{b:02X}' for b in DATA)}")
            time.sleep(INTERVAL)
    except KeyboardInterrupt:
        pass
    finally:
        print(f"\nSent {count} frames total. Shutting down...")
        bus.shutdown()

    return 0

if __name__ == "__main__":
    sys.exit(main())
