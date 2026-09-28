/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Overview
 * --------
 * A small FreeRTOS demo for the Pico W with two tasks:
 *
 *   main_task  - a USB-serial "case swapper". Every character received on
 *                stdin is echoed back with its letter case inverted
 *                ('a' -> 'A', 'Q' -> 'q'); anything else is echoed unchanged.
 *   blink_task - blinks the on-board LED (wired to the CYW43 wireless chip,
 *                not a regular RP2040 GPIO) every 500 ticks, except that the
 *                toggle is skipped once every 11 iterations, giving an
 *                irregular blink pattern.
 *
 * Execution contexts:
 *   main()      - runs once on reset, creates main_task, then hands the CPU to
 *                 the FreeRTOS scheduler. vTaskStartScheduler() never returns.
 *   main_task   - FreeRTOS task (priority idle+1), spawns blink_task and then
 *                 loops forever blocking on getchar().
 *   blink_task  - FreeRTOS task (priority idle+2), loops forever.
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

// Shared state for blink_task. Globals, so they persist across loop
// iterations and are visible to the debugger. Only blink_task touches them.
int count = 0;      // number of blink iterations so far
bool on = false;    // current LED state to write on the next iteration

// Blink runs at a higher priority than the serial echo so the LED keeps its
// timing even while main_task is busy.
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void blink_task(__unused void *params) {
    // The LED is behind the CYW43 radio, so the radio driver must be up
    // before we can drive it. It is initialised here (inside a task) because
    // the FreeRTOS flavour of cyw43_arch requires the scheduler to be running.
    hard_assert(cyw43_arch_init() == PICO_OK);
    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
        // Toggle on every iteration except when count is a multiple of 11
        // (including the very first iteration, count == 0). Note count is
        // post-incremented, so the test uses the value *before* the increment.
        if (count++ % 11) on = !on;
        // 500 ticks; with configTICK_RATE_HZ = 1000 this is 500 ms.
        vTaskDelay(500);
    }
}

void main_task(__unused void *params) {
    xTaskCreate(blink_task, "BlinkThread",
                BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
    char c;
    // getchar() blocks until a character arrives over USB serial. The loop
    // only exits if a NUL byte (0) is received, after which the task function
    // returns -- which FreeRTOS does not allow, so in practice treat that as a
    // bug rather than a feature.
    while(c = getchar()) {
        // ASCII upper and lower case letters differ by 32 ('a' - 'A' == 32).
        if (c <= 'z' && c >= 'a') putchar(c - 32);
        else if (c >= 'A' && c <= 'Z') putchar(c + 32);
        else putchar(c);
    }
}

int main( void )
{
    // Bring up USB CDC serial so getchar()/putchar() work.
    stdio_init_all();
    const char *rtos_name;  // unused leftover from the SDK example
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);
    // Start FreeRTOS. From here on the scheduler owns the CPU and this call
    // does not return unless there was not enough heap to start.
    vTaskStartScheduler();
    return 0;
}
