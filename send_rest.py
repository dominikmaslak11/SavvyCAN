#!/usr/bin/env python3
"""
SavvyCAN REST API Frame Sender
Sends extended CAN frame 0x1421003F every 10 seconds through
SavvyCAN's built-in REST API (POST /api/send).

Requires:
  - SavvyCAN running with REST API enabled (Settings -> REST API -> Enable)
  - Python 3.6+ (stdlib only, no pip packages needed)
"""

import urllib.request
import urllib.error
import json
import time
import signal
import sys

# --- Configuration ----------------------------------------------------
SAVVYCAN_URL = "http://localhost:8080"
FRAME_ID = 0x1421003F          # Extended CAN ID
DATA = [0x50, 0x01, 0x00, 0x00, 0x00, 0x32, 0x19, 0xB1]
INTERVAL = 10.0                # seconds
BUS = 0                        # SavvyCAN bus number
PCAN_DRIVER = "peakcan"        # Qt SerialBus driver name
PCAN_PORT = "PCAN_USBBUS1"     # PCAN device port
BITRATE = 250000               # CAN bus speed

running = True

def signal_handler(sig, frame):
    global running
    print("\nStopping...")
    running = False

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def connect_can():
    """Connect SavvyCAN to a PCAN interface. Returns (ok, message)."""
    payload = json.dumps({
        "driver": PCAN_DRIVER,
        "port": PCAN_PORT,
        "bitrate": BITRATE
    }).encode()

    try:
        req = urllib.request.Request(
            f"{SAVVYCAN_URL}/api/connect",
            data=payload,
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        with urllib.request.urlopen(req, timeout=2) as resp:
            body = resp.read().decode()
            return True, body
    except urllib.error.URLError as e:
        return False, str(e.reason)
    except Exception as e:
        return False, str(e)


def send_frame():
    """Send one frame via REST API. Returns (ok, message)."""
    payload = json.dumps({"id": FRAME_ID, "bus": BUS, "data": DATA}).encode()

    try:
        req = urllib.request.Request(
            f"{SAVVYCAN_URL}/api/send",
            data=payload,
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        with urllib.request.urlopen(req, timeout=2) as resp:
            body = resp.read().decode()
            return True, body
    except urllib.error.URLError as e:
        if hasattr(e, 'reason') and 'refused' in str(e.reason).lower():
            return False, "Connection refused - is SavvyCAN running with REST API enabled?"
        return False, str(e.reason)
    except Exception as e:
        return False, str(e)

def main():
    print("SavvyCAN REST API Frame Sender")
    print(f"  Target: {SAVVYCAN_URL}/api/send")
    print(f"  ID:     0x{FRAME_ID:08X} (extended)")
    print(f"  DLC:    {len(DATA)}")
    print(f"  Data:   {' '.join(f'{b:02X}' for b in DATA)}")
    print(f"  Bus:    {BUS}")
    print(f"  Every:  {INTERVAL}s")
    print(f"  Press Ctrl+C to stop")
    print()

    # Step 1: Connect to PCAN
    print("Connecting to PCAN...")
    ok, msg = connect_can()
    if not ok:
        print(f"WARNING: Could not connect PCAN: {msg}")
        print("Frames will be logged but NOT transmitted on the bus.")
        print("Make sure PCAN hardware is connected and drivers are installed.")
    else:
        print(f"Connected: {msg}")

    # Step 2: Test send
    print("Testing frame send...")
    ok, msg = send_frame()
    if not ok:
        print(f"ERROR: {msg}")
        print()
        print("Make sure:")
        print("  1. SavvyCAN is running")
        print("  2. REST API is enabled (Settings -> Enable REST API)")
        print("  3. Port 8080 is not blocked")
        return 1

    resp_json = json.loads(msg) if msg.startswith("{") else {}
    transmitted = resp_json.get("transmitted", "?")
    print(f"[1] Sent -> stored: ok, transmitted: {transmitted}")

    count = 1
    try:
        while running:
            time.sleep(INTERVAL)
            if not running:
                break
            ok, result = send_frame()
            count += 1
            if ok:
                resp_j = json.loads(result) if result.startswith("{") else {}
                tx = resp_j.get("transmitted", "?")
                print(f"[{count}] Sent -> stored: ok, transmitted: {tx}")
            else:
                print(f"[{count}] ERROR: {result}")
    except KeyboardInterrupt:
        pass

    print(f"\nSent {count} frames total. Done.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
