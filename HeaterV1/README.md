<div align="center">

# HeaterV1 Firmware

ESP32-based dual‑channel heater controller with USB Power Delivery (PD) negotiation, PID temperature control, LVGL UI, persistent settings, and runtime statistics.

</div>

---

## ✨ Feature Overview
* Dual independent temperature sensing & PID control (currently shared target; architecture supports per‑channel later)
* USB‑PD fixed voltage negotiation (5V/9V/15V/20V) with startup capability snapshot + runtime voltage switching (START→20/15V, STOP→5V)
* Optimized LVGL user interface (bitmap large digits font, subjects data binding, inactivity sleep/dim, blinking edit cursor)
* Settings persistence (NVS) with debounced / batched writes and versioned structure
* Live target temperature adjustments while heating (no need to STOP)
* Heater runtime accumulation (minutes persisted; UI shows whole hours)
* Safety: over‑temperature cutoff, minimum screen brightness enforcement, fallback PD handling
* Modular task separation (Director, Fireman, Mailman, Switchboard, Wiseman, Composer)

---
## � AI‑Assisted Development Note
This project intentionally experiments with an AI‑assisted workflow. Portions of code, refactors, and documentation have been drafted or refined with the help of ChatGPT / Copilot‑style tooling, then reviewed and integrated manually. Goals of this experiment:
* Evaluate productivity gains for embedded + RTOS + UI integration work.
* Identify where AI suggestions are strong (structuring modules, boilerplate reduction, documentation) vs. where human oversight is critical (hardware interactions, timing, safety, resource constraints).
* Establish lightweight guardrails: review every hardware‑touching change, enforce style coherency, and run builds after non‑trivial edits.

Heuristic provenance metrics can be regenerated anytime:
```bash
python scripts/code_origin_stats.py
```
(Results are approximate; they rely on pattern matching, not a cryptographic history.)

If you contribute, please continue to: (1) treat AI output as a draft, (2) preserve safety checks, and (3) prefer clarity over cleverness.

---
## �🧱 Architecture Summary

| Module | Responsibility | Key Files |
|--------|----------------|-----------|
| **app_main** | Bootstraps subsystems, creates queues, snapshots PD capabilities, launches Director | `main.c` |
| **Director** | UI orchestration, button event handling, page state machine, PD voltage requests, heater START/STOP logic, runtime display updates | `director/director.c` |
| **Fireman** | Periodic ADC sampling, PID loops, heater PWM duty computation, temperature sample publication, minute runtime accumulation | `fireman/fireman.c` |
| **Mailman (I2C Task)** | Serialized I2C access; executes ADC reads, PD SET_PDO commands, ambient sensor queries | `mailman/i2c_task.c` + device drivers |
| **Switchboard** | Button (user input) detection + PD capability helpers | `switchboard/user_input.c`, `switchboard/pd_caps.c` |
| **Wiseman** | Persistent settings (setpoints, brightness, sleep timeout, runtime minutes, etc.) + dirty tracking + background save | `wiseman/wiseman.c`, `wiseman_persist.c` |
| **Composer** | Buzzer / audible feedback (future expansion) | `composer/buzzer.*` |
| **UI Assets** | Generated LVGL layout & subjects; volatile runtime bindings (NOT manually edited) | `director/HeaterGUI/*` |

### Data Flow
1. Buttons → `Switchboard` → queue → `Director` (events)  
2. `Director` adjusts LVGL subjects & invokes `Fireman` setters (enable heaters, update setpoints).  
3. `Fireman` requests ADC via `Mailman` → receives temps → updates PWM duties → publishes latest sample (single‑slot queue).  
4. `Director` consumes temperature sample → updates UI subjects (quantized).  
5. On START/STOP `Director` requests PD voltage via `Fireman` wrapper (which forwards to `Mailman`).  
6. `Wiseman` persists updated settings / runtime minutes through background debounce.

### Tasks (FreeRTOS)
| Task | Approx Function | Notes |
|------|------------------|-------|
| Director | UI loop (lv_timer_handler, events) | 10 ms tick delay, lightweight polling |
| Fireman | Control loop (500 ms active / 1500 ms idle) | Adaptive period lowers power |
| I2C Task | Serialized device communication | Responds to queued messages |
| (Persistence) | Wiseman background saver | Debounced writes |

---
## 🔌 USB Power Delivery Strategy
* PD capabilities read once at startup (snapshot).  
* START: request 20V, fallback to 15V if 20V unsupported or negotiation fails.  
* STOP: revert to 5V.  
* Active voltage stored in `activePDO` subject for UI.

---
## ♨️ Heater Control
* Two LEDC PWM channels (13‑bit) at 1 kHz.  
* PID: tuned gains (P=2.0, I=0.03, D=4.0) with derivative clamp and anti‑windup.  
* Over‑temperature threshold: 60 °C (disables both channels + resets integrators).  
* Runtime accumulation counts minutes while any heater channel is enabled (independent of duty) and persists every elapsed minute.

---
## 🖥 UI / UX Notes
* LVGL subjects: temperature digits separated (whole + decimal), PD availability flags, brightness, sleep timer, run time hours, etc.  
* Large digits font is a pre‑rasterized bitmap to avoid runtime TTF stalls.  
* Inactivity sleep dims backlight to 0 (but stored brightness preserved, min enforced at 5% on wake).  
* Live target temperature changes propagate instantly to PID when heating.

---
## 💾 Persistence (Wiseman)
* Versioned struct (`WISEMAN_SETTINGS_VERSION`).  
* Fields: setpoints, heater enable flags, sound flag, brightness, Wi‑Fi creds (future), accumulated op minutes, sleep timeout.  
* Dirty tracking snapshots to avoid redundant NVS writes.  
* Operating time increments marked dirty at most once/minute (wear conscious).

---
## 🚀 Build & Flash

Prerequisites: ESP-IDF installed & export script sourced.

Typical workflow:
```bash
idf.py build
idf.py -p <PORT> flash monitor
```
On Windows PowerShell (after calling exported ESP-IDF environment script):
```powershell
idf.py build
idf.py -p COM5 flash monitor
```

To exit monitor: `Ctrl+]` (or `Ctrl+T` then `Ctrl+Q` depending on IDF version).

---
## 📁 Key Directory Layout
```
main/
	main.c                # Bootstrap, queue creation, PD caps snapshot, director start
	director/             # UI controller + generated LVGL assets
	fireman/              # Heater control task (PID + ADC integration)
	mailman/              # I2C task + device drivers (PD controller, ADC, temperature sensor)
	switchboard/          # User input & PD caps helpers
	wiseman/              # Persistent settings & runtime accumulation
	composer/             # Buzzer (sound) logic
	lv_conf.h             # LVGL configuration
```

---
## 🔐 Safety / Guardrails
* Over‑temperature forced shutdown.  
* Invalid PD voltage requests filtered.  
* Minimum brightness enforced (≥5%) to avoid UI lockout.  
* ADC failure path forces heaters off and emits NaN sample (UI zeros display).

---
## 📊 Runtime Statistics
* `runTime` subject (hours) derived from persisted minutes.  
* Future: could expose per‑channel on‑time, duty cycle histogram, energy estimation.

---
## 🛣 Roadmap Ideas (Not Yet Implemented)
* Independent per‑channel setpoint UI & persistence
* Visual heater active indicators (duty > 0%)
* Over‑temperature notification subject + UI alert
* Energy usage estimation (Wh) using PD voltage & approximate current
* Wi‑Fi + remote telemetry / OTA updates
* Advanced PID tuning page and auto‑tune routine
* PD dynamic capability re‑query / renegotiation resilience

---
## 🔧 Development Utilities
* `scripts/code_origin_stats.py` – heuristic line origin statistics (ai/tool/human).  
* Single‑slot queues (temperature sample) minimize latency and avoid backlog drains.

---
## 🧪 Testing Strategy (Current / Planned)
Current: manual functional verification via log output & UI.  
Planned: component tests for PID edge cases, persistence cycle tests, simulated ADC fault injection.

---
## 🙌 Contributing
When editing generated LVGL files (`*_gen.*`), prefer adjusting the source XML or generation pipeline—avoid manual edits that will be overwritten.

---
## License
TBD (specify license once chosen).

---
## Changelog Snapshot (Recent Highlights)
* Added heater runtime accumulation (minutes→hours UI)
* Live target temperature adjustments while running
* Centralized PD voltage negotiation in Director (start/stop) with fallback
* Optimized Fireman loop (adaptive period, reduced LEDC churn)
* Added inactivity sleep/dim & min brightness safeguard
* Replaced TTF digits with bitmap font to eliminate frame stalls

---
_This README reflects the current architecture as of the latest commit. Update when adding major features._
