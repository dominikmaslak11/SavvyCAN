#!/usr/bin/env python3
"""
SavvyCAN Python Bridge Frame Sender
Use this script inside SavvyCAN's built-in Python console
(Scripting → Python Console).

Paste the entire content, or import as a module and call run_loop().

The savvycan module is available automatically inside SavvyCAN's
embedded Python interpreter.
"""

import time

# ── Configuration ────────────────────────────────────────────────────
FRAME_ID = 0x1421003F          # Extended CAN ID
DATA = [0x50, 0x01, 0x00, 0x00, 0x00, 0x32, 0x19, 0xB1]
INTERVAL = 10.0                # seconds
BUS = 0                        # SavvyCAN bus number


def send_one():
    """Send a single frame through SavvyCAN's connected buses."""
    savvycan.send_frame(id=FRAME_ID, data=DATA, bus=BUS)
    print(f"Sent 0x{FRAME_ID:08X}  {' '.join(f'{b:02X}' for b in DATA)}")


def run_loop():
    """Send the frame repeatedly every INTERVAL seconds. Press Stop in the console to abort."""
    print(f"SavvyCAN Bridge Sender")
    print(f"  ID:     0x{FRAME_ID:08X} (extended)")
    print(f"  DLC:    {len(DATA)}")
    print(f"  Data:   {' '.join(f'{b:02X}' for b in DATA)}")
    print(f"  Bus:    {BUS}")
    print(f"  Every:  {INTERVAL}s")
    print(f"  Press 'Stop' button in SavvyCAN to abort\n")

    count = 0
    while True:
        try:
            savvycan.send_frame(id=FRAME_ID, data=DATA, bus=BUS)
            count += 1
            print(f"[{count}] Sent 0x{FRAME_ID:08X}")
            time.sleep(INTERVAL)
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"[{count}] ERROR: {e}")
            time.sleep(INTERVAL)

    print(f"\nSent {count} frames total. Done.")


# ── One-shot mode ────────────────────────────────────────────────────
if __name__ == "__main__":
    # When run as a standalone script (for testing), just print info
    print("This script is meant to run inside SavvyCAN's Python Console.")
    print("Open SavvyCAN → Scripting → Python Console, then paste:")
    print()
    print(f"  FRAME_ID = {FRAME_ID}")
    print(f"  DATA = {DATA}")
    print(f"  savvycan.send_frame(id=FRAME_ID, data=DATA, bus={BUS})")
    print()
    print("For a continuous loop, use run_loop().")
