# Jackson-Embedded-Lab

A personal embedded software playground for practicing Linux C development, with a focus on small tools relevant to the automotive electronics industry (vehicle ECU communication, diagnostics, and log analysis).

This repository is primarily a learning project. Each tool started as a way to deepen my understanding of a specific concept — file I/O, CLI argument parsing, CMake, CRC algorithms, CAN protocol formats — and has been iteratively debugged and refined across multiple sessions.

## Repository Structure

```
Jackson-Embedded-Lab/
├── Applications/
│   ├── CANAnalyzer/       # CAN frame analyzer (file / CLI / socket input) — in progress
│   ├── CRCCalculator/     # CRC32 (IEEE 802.3) calculator
│   ├── Common/            # Shared headers (INTERFACE library)
│   ├── HexTool/           # hexdump -C style viewer + hex-string/binary converter
│   ├── LogAnalyzer/       # Vector CANoe ASC log parser (Classical CAN & CAN FD)
│   ├── TestApp/           # Test harness / executables for module validation
│   ├── third_party/       # External dependencies
│   └── tools/             # Supporting scripts and utilities
└── Project/
    ├── CMakeLists.txt     # Root CMake build configuration
    ├── build.sh           # Build script (Debug/Release modes, clean command)
    └── README.md
```

## Tools Overview

| Tool | Description |
|---|---|
| **LogAnalyzer** | Currently, support read file, case fold, key word search |
| **HexTool** | A `hexdump -C` style utility for viewing binary data, plus a hex-string-to-binary converter. |
| **CRCCalculator** | Computes CRC32 (IEEE 802.3), validated against the standard test vector (`123456789 → CBF43926`). |
| **CANAnalyzer** *(in progress)* | Parses raw CAN frame data into structured CAN Frame objects. Designed to accept input from multiple sources — a file, the command line, or a socket/port input buffer — and analyze it into decoded CAN frames. |
| **Common** | Shared header library used across modules (CMake `INTERFACE` target). |
| **TestApp** | Executables used to validate individual modules during development, and test data |

> Each `Applications/*` module typically builds as a static library (`.a`) paired with a corresponding test executable, following a consistent module pattern across the repo.

## Tech Stack

- **Language:** C (Linux)
- **Build system:** CMake (`file(GLOB)` auto-discovery, `add_subdirectory` per module, `PUBLIC`/`PRIVATE` include scoping)
- **Environment:** Ubuntu (Linux) virtual machine
- **Version control:** Git

## Building

```bash
cd Project
./build.sh Debug     # or Release
```

Use `./build.sh clean` to remove build artifacts.

## Background & Motivation

I work in automotive embedded software development, with a focus on connected vehicle systems (T-Box, IVI) and vehicle communication protocols (CAN, CAN FD, UDS, DoIP, SOME/IP) within the AUTOSAR Classic Platform ecosystem. This repository exists to sharpen my Linux C fundamentals and explore how the concepts I use professionally — protocol parsing, communication stacks, diagnostic tooling — can be built up from first principles.

## Status

Actively evolving. Current focus areas:
- Extending CANAnalyzer's parsing logic toward a state-machine-based architecture
- Building out CANAnalyzer to support three input modes — file input, command-line input, and a socket/port input buffer — all analyzed into a common CAN Frame representation

## License

Personal project — no license specified yet.
