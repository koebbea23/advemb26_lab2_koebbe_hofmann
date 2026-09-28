/*
 * FreeRTOS task entry points. They only handle *how* things run (loops,
 * delays, I/O, drivers). *What* they do lives in blink.c and case_swap.c,
 * which have no hardware dependencies and are unit tested in test/.
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "tasks.h"
#include "blink.h"
#include "case_swap.h"

// The Pico W LED is wired to the CYW43 radio, not to an RP2040 GPIO.
static void cyw43_led_put(bool on)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

void blink_task(__unused void *params) {
    // The FreeRTOS flavour of cyw43_arch needs the scheduler running, so the
    // driver is initialised here rather than in main().
    hard_assert(cyw43_arch_init() == PICO_OK);
    int count = 0;
    bool on = false;
    while (true) {
        on = blink_step(on, &count, cyw43_led_put);
        vTaskDelay(BLINK_DELAY_TICKS);
    }
}

void main_task(__unused void *params) {
    xTaskCreate(blink_task, "BlinkThread",
                BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
    char c;
    // getchar() blocks until a character arrives over USB serial. The loop
    // only exits on a NUL byte, and then the task returns, which FreeRTOS
    // does not allow. This is kept from the original code on purpose; see
    // test/manual/blink_and_echo.md.
    while ((c = getchar())) putchar(switch_case(c));
}
