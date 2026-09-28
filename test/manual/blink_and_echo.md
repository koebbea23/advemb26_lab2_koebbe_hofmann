# Manual regression test: LED blink and serial case swap

Covers the two behaviors of `hello_freertos`: the on-board LED blink pattern
and the USB-serial case-swapping echo. Run this before and after any change to
`src/` to confirm behavior is unchanged. Takes about 5 minutes.

## 1. Setup

Equipment:
- Raspberry Pi Pico W (the target) and a micro-USB data cable.
- A host with `picotool` installed and a serial terminal (`minicom`, `screen`,
  `tio`, PuTTY, or the VS Code Serial Monitor extension).
- A stopwatch or phone timer.

Steps:
1. Build the project: `cmake -B build -S . && cmake --build build`
   (or **Compile Project** from the Raspberry Pi Pico extension).
2. Hold BOOTSEL on the Pico W and plug it in. It enumerates as a mass-storage
   device `RPI-RP2`.
   - WSL only: attach it from Windows with
     `usbipd attach --wsl --busid <BUSID>` (find the bus id with `usbipd list`).
3. Flash: `picotool load -x build/src/hello_freertos.elf -f`
   (or drag `build/src/hello_freertos.uf2` onto `RPI-RP2`).
4. The board reboots and a serial port appears (`/dev/ttyACM0` on Linux,
   `COMx` on Windows, `/dev/tty.usbmodem*` on macOS). On WSL, re-run the
   `usbipd attach` command because the device re-enumerates.
5. Open the port, for example `tio /dev/ttyACM0` or
   `minicom -D /dev/ttyACM0 -b 115200`. The baud rate does not matter for USB
   CDC. **Turn local echo off** so you only see what the Pico sends back.

## 2. Exercise the system and 3. Expected behavior

| # | Action | Expected result |
|---|--------|-----------------|
| 1 | Watch the LED for about 30 seconds without touching anything. | The LED blinks at a steady rate of 0.5 s ON and 0.5 s OFF. Every 5.5 s there is one **longer OFF gap of about 1 s** (the "hiccup"). Nothing else is irregular. |
| 2 | Time 10 hiccups with the stopwatch. | About 55 s (10 × 11 × 0.5 s). |
| 3 | Type `abcxyz`. | Terminal shows `ABCXYZ`. |
| 4 | Type `ABCXYZ`. | Terminal shows `abcxyz`. |
| 5 | Type `Hello, World!` | Terminal shows `hELLO, wORLD!` |
| 6 | Type `0123456789 !@#$%^&*()_-+=[]{};:'",.<>/?\|~` and a backtick. | Echoed back unchanged. |
| 7 | Type the boundary characters `` @ A Z [ ` a z { `` | Terminal shows `` @ a z [ ` A Z { `` (only letters change). |
| 8 | Paste a long line (about 200 characters) quickly. | Every character is echoed with its case swapped and none are dropped. The LED keeps blinking normally while you paste. |
| 9 | Press Enter. | The cursor returns to the start of the line (CR is echoed). |
| 10 | Close the terminal, wait 5 s, and reopen it. Type `q`. | `Q` is echoed. The LED never stopped blinking. |
| 11 | Unplug the board and plug it back in (not in BOOTSEL). | The LED starts blinking within about 1 s, and steps 3–4 work again after reconnecting. |

A pass requires every row to match. Record any deviation, the build commit
(`git rev-parse --short HEAD`), and the board used.

## Known edge cases not covered

- Sending a NUL byte (0x00) makes `main_task` return from its loop. That is
  undefined behavior in FreeRTOS, and this plan does not test it.
- Characters outside ASCII (UTF-8 multibyte) are echoed byte-for-byte. Their
  display depends on the terminal.
