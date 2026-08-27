# SEDS Avionics Induction Submission

This repository contains my completed solutions for the SEDS Avionics subsystem induction evaluation. 

Below is a breakdown of how I tackled each task, along with the hardware troubleshooting and code adaptations made during development.

---

## Task 1: Python Depth Monitor (`Task1_Avionics/depth_monitor.py`)

### Overview
A Python telemetry script designed to parse depth and altitude data streams in real time and trigger system warning flags when preset safe thresholds are exceeded.

### Implementation & Troubleshooting
* **Telemetry Processing:** Structured the script to ingest real-time numerical inputs and compare them against upper and lower threshold constraints.
* **Environment & File Handling:** Resolved initial Windows extension masking issues (where the file was named `depth_monitor.py.py`) and normalized line endings across Git using standard CRLF/LF conversions.
* **Alert System:** Designed formatted terminal status logs so telemetry limits are clearly flagged during live execution.

---

## Task 2: Athena Hardware System (`Task2_Avionics/athena_monitor.ino`)

### Hardware Debugging & Circuit Adaptation
* **Push Button Issue:** The initial circuit design utilized a momentary push button to toggle system operational states. During simulation testing, this caused persistent switch-bouncing issues and unexpected logic loops, leading to false state triggers.
* **Component Switch:** To eliminate floating pin behavior and improve control stability, I removed the push button and integrated a 3-pin slide switch into the circuit layout.
* **Firmware Adaptation (`athena_monitor.ino`):** Rewrote the input read routines in C++/Arduino to handle persistent high/low logic levels rather than transient pulse edge detection. This simplified the control architecture and removed the need for software debounce routines.
* **Schematic Verification:** Mapped out and validated the revised hardware connections in Tinkercad, saving the verified layout as `tinkercad_wiring.png`.

---

## Repository Layout

```text
├── Task1_Avionics/
│   └── depth_monitor.py       # Python telemetry depth processing script
├── Task2_Hardware/
│   ├── athena_monitor.ino     # Updated Arduino firmware for slide switch logic
│   └── tinkercad_wiring.png   # Complete Tinkercad schematic & circuit wiring
└── README.md                  # Development log and documentation
