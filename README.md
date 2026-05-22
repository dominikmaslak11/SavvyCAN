# SavvyCAN
Qt based cross platform canbus tool
(C) 2015-2026 Collin Kidder

A Qt6 based cross platform tool which can be used to load, save, and capture canbus frames.
This tool is designed to help with visualization, reverse engineering, debugging, and
capturing of canbus frames.

Please use the "Discussions" tab here on GitHub to ask questions and interact with the community.

Requires a resolution of at least 1024x768. Fully multi-monitor capable. Works on 4K monitors as well.

You are highly recommended to use the
[CANDue board from EVTV](http://store.evtv.me/proddetail.php?prod=ArduinoDueCANBUS&cat=23).

The CANDue board must be running the GVRET firmware which can also be found
within the collin80 repos.

It is now possible to use any Qt SerialBus driver (socketcan, Vector, PeakCAN, TinyCAN).

It should, however, be noted that use of a capture device is not required to make use
of this program. It can load and save in several formats:

1. BusMaster log file
2. Microchip log file
3. CRTD format (OVMS log file format from Mark Webb-Johnson)
4. GVRET native format
5. Generic CSV file (ID, D0 D1 D2 D3 D4 D5 D6 D7)
6. Vector Trace files
7. IXXAT Minilog files
8. CAN-DO Logs
9. Vehicle Spy log files
10. CANDump / Kayak (Read only)
11. PCAN Viewer (Read Only)
12. Wireshark socketcan PCAP file (Read only)

## Dependencies

Now this code does not depend on anything other than what is in the source tree or available
from the Qt installer.

Uses QCustomPlot available at:

http://www.qcustomplot.com/

However, this source code is integrated into the source for SavvyCAN and one isn't required
to download it separately.

This project requires Qt 6.2 or higher.

## Instructions for compiling (CMake + Qt6):

[Download the newest stable version of Qt directly from qt.io](https://www.qt.io/download/) (You need Qt 6.2 or newer)

```sh
cd ~

git clone https://github.com/collin80/SavvyCAN.git

cd SavvyCAN

cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 -DCMAKE_BUILD_TYPE=Release -GNinja

cmake --build build -j$(nproc)
```

Now run SavvyCAN

```
./SavvyCAN
```

## What to do if your compile failed?

The very first thing to do is try:

```sh
rm -rf build

cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 -GNinja

cmake --build build
```

Did that fix it? Great! If not, ensure that you selected SerialBUS support
when you installed Qt.

### What to do if CMake fails with missing Qt modules on Ubuntu/Debian?

`sudo apt install qt6-serialbus-dev qt6-serialport-dev qt6-declarative-dev qttools6-dev`

### Used Items Requiring Attribution

nodes by Adrien Coquet from the Noun Project

message by Vectorstall from the Noun Project

signal by shashank singh from the Noun Project

signal by juli from the Noun Project

signal by yudi from the Noun Project

Death by Adrien Coquet from the Noun Project