# AGENTS.md — Hardware & Firmware Surface

> Surface directive for ESP32-S3 Firmware, KiCad PCB, and Test Bench Lab.
> Parent governance: [`../AGENTS.md`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/AGENTS.md) and [`../harness/harness.config.yaml`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/harness/harness.config.yaml).

## 1. Surface Architecture
- **Firmware Core**: C++ object-oriented modules in [`src/`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/src/) (`core/`, `button/`, `led/`, `buzzer/`, `power/`, `ble/`).
- **All-in-one Sketch**: [`SmartOrderButton/SmartOrderButton.ino`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/SmartOrderButton/SmartOrderButton.ino) (must always stay synchronized with modular code).
- **Pin & Mode Configurations**: Centralized strictly in [`include/config.h`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/include/config.h).
- **Hardware Layout**: [`Smart_Order_Button.kicad_sch`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/Smart_Order_Button.kicad_sch) & [`Smart_Order_Button.kicad_pcb`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/Smart_Order_Button.kicad_pcb).
- **Test Bench**: [`test_bench/index.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/test_bench/index.html) and automated runner [`test_bench/run_test.js`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/test_bench/run_test.js).

## 2. Hard Requirements for Firmware Agents
1. **Zero GPIO Hardcoding**: Never write numeric pin numbers directly in business logic. Always use `BUTTON_ORDER_PIN`, `BUTTON_BILL_PIN`, `BUTTON_CANCEL_PIN`, `LED_R_PIN`, `LED_G_PIN`, `LED_B_PIN`, `BUZZER_PIN`.
2. **Dual-Variant Decoders**:
   - `HARDWARE_VARIANT_SINGLE_BUTTON (1)`: Single button gesture decode (`detectSingleButtonGesture`).
   - `HARDWARE_VARIANT_TRIPLE_BUTTON (3)`: Triple direct button decode (`detectTripleButtonGesture`).
3. **Power & Sleep Safety**:
   - Always ensure `gpio_wakeup_enable` is registered for all active button pins prior to `esp_light_sleep_start()`.
   - Never block FreeRTOS loops without calling `vTaskDelay` or `delay()` to prevent watchdog triggers.
4. **Header Guards**: Every `.h` file must include guards `#ifndef / #define / #endif`.

## 3. Hard Requirements for PCB/Hardware Agents
1. **ESP32-S3 RF Keepout**: Maintain strict zero-copper zone beneath and around the PCB antenna.
2. **Signal Integrity & Debounce**: Preserve pull-up resistors and RC debounce on tactile switches SW1, SW2, SW3.
3. **Power Budget**: TP4065 charging current set via R_PROG must not exceed battery rating (<= 500mA for 1000mAh LiPo).

## 4. Verification Gate
Before completing any edit in `hardware/`, run:
```powershell
node hardware/test_bench/run_test.js
```
Expected result: **All 14 tests passing (100%)**.
