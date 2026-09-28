#pragma once

#include <stdbool.h>

// Number of blink iterations per pattern period. On the iteration where the
// count is a multiple of this, the LED is not toggled, which leaves one
// double-length gap per period.
#define BLINK_PERIOD 11

/*
 * Something that drives the LED. It is passed in rather than called
 * directly, so the blink logic does not depend on the CYW43 driver and tests
 * can supply a fake that records what was written.
 */
typedef void (*led_put_fn)(bool on);

/*
 * Pure blink logic: returns the LED state for the next iteration and
 * advances *count by one. The state toggles unless *count (the value
 * before the increment) is a multiple of BLINK_PERIOD.
 *
 *   on    - current LED state
 *   count - in/out iteration counter
 */
bool blink_next_state(bool on, int *count);

/*
 * One iteration of the blink loop: writes `on` to the LED through `led_put`,
 * then returns the state for the next iteration (see blink_next_state).
 */
bool blink_step(bool on, int *count, led_put_fn led_put);
