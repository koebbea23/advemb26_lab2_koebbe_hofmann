# advemb26_lab2_koebbe_hofmann

Lab 2: Writing testable code. A FreeRTOS demo for the Pico W, refactored so
its logic can be unit tested.

## What the firmware does
- **LED blink**: the on-board LED toggles every 500 ms, except once every 11
  iterations, which gives one 1 s OFF gap every 5.5 s.
- **Serial case swap**: each character typed over USB serial is echoed back
  with its letter case swapped (`Hello` → `hELLO`).

## Layout
| Path | Contents |
|------|----------|
| `src/hello_freertos.c` | `main()`: starts stdio, creates `main_task`, starts the scheduler |
| `src/tasks.c`, `include/tasks.h` | FreeRTOS task entry points (loops, delays, I/O) |
| `src/blink.c`, `include/blink.h` | Pure blink logic, `blink_next_state()`, plus `blink_step()` with an injected LED setter |
| `src/case_swap.c`, `include/case_swap.h` | `switch_case()` |
| `src/led.c`, `include/led.h` | Thin wrapper around the CYW43 LED |
| `test/test_logic.c` | Unity unit tests (target `mytest`). No hardware dependencies |
| `test/test_blink_hw.c` | Hardware integration test for the CYW43 LED (target `blink_hwtest`, Pico W only) |
| `test/manual/blink_and_echo.md` | Manual regression test plan |
| `lib/` | Submodules: pico-sdk, FreeRTOS-Kernel, Unity |

## Build
```sh
git submodule update --init --recursive
cmake -B build -S .
cmake --build build -j
```
In VS Code, use **Compile Project** from the Raspberry Pi Pico extension.
Outputs: `build/src/hello_freertos.{elf,uf2}`, `build/test/mytest.{elf,uf2}`,
`build/test/blink_hwtest.{elf,uf2}`.

## Flash and run tests on hardware
1. Hold BOOTSEL and plug in the Pico W.
   - WSL: attach the device from an admin PowerShell:
     `usbipd list`, `usbipd bind --busid <ID>` (once), then
     `usbipd attach --wsl --busid <ID>`. Re-attach after every reboot or re-enumeration.
2. Load and run a binary:
   ```sh
   picotool load -x build/test/mytest.elf -f
   ```
   or `cmake --build build --target flash_test` (for the hardware test:
   `--target flash_blink_hwtest`, for the app: `--target flash`).
   In VS Code, set the CMake Launch/Debug target and run the **Run Project**
   task (picotool) or **Flash** (OpenOCD with a debug probe).
3. Open the serial port (`tio /dev/ttyACM0`) to see the Unity report, ending
   with `N Tests 0 Failures 0 Ignored OK`. The test binaries rerun every 5 s,
   so a late-attaching port still catches a full run. `picotool load -f`
   reboots the board into BOOTSEL to reflash it.
   - WSL: the USB serial driver must be loaded (`sudo modprobe cdc-acm`,
     made persistent with `echo cdc-acm | sudo tee /etc/modules-load.d/cdc-acm.conf`),
     and `usbipd attach --wsl --busid <ID> --auto-attach` must be running.
     Install `picotool/udev/60-picotool.rules` into `/etc/udev/rules.d/`
     so picotool works without sudo.

## Run tests in simulation
Needs Renode (see the setup below). Run `ctest --test-dir build`, or use the
**Simulate** task with `mytest` selected as the launch target. Only `mytest`
runs in Renode because the CYW43 is not simulated.

## Renode setup
The Raspberry Pico needs configuration files for Renode to work properly.

* On MacOS, the installation location is `/Applications/Renode.app/Contents/MacOs`
* On Linux, the location for Debian, Fedora, and Arch is `/opt/renode`
* On Windows, the location is `C://Program Files/Renode`

To add the Pico configuration files:
1. Copy `rp2040_spinlock.py` and `rp2040_divider.py` to the `scripts/pydev` directory of your Renode installation.
1. Copy `rpi_pico_rp2040_w.repl` to the `platforms/cpus` directory.
