#include "blink.h"

bool blink_next_state(bool on, int *count)
{
    // Post-increment on purpose: the original loop tested `count++ % 11`,
    // so the check uses the value before the increment. Changing this would
    // shift the position of the long gap by one blink.
    return ((*count)++ % BLINK_PERIOD) ? !on : on;
}

bool blink_step(bool on, int *count, led_put_fn led_put)
{
    led_put(on);
    return blink_next_state(on, count);
}
