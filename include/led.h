#pragma once

#include <stdbool.h>

/*
 * Pico W on-board LED. The LED is wired to the CYW43 radio, not to an
 * RP2040 GPIO, so the CYW43 driver must be initialised (cyw43_arch_init)
 * before these are called.
 */

// Drive the LED. Matches led_put_fn in blink.h so it can be injected.
void led_put(bool on);

// Read back the LED state from the CYW43.
bool led_get(void);
