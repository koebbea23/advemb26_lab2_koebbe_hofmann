#pragma once

#include "FreeRTOS.h"

// Blink runs at a higher priority than the serial echo so the LED keeps its
// timing even while main_task is busy.
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

// Ticks between blink iterations. With configTICK_RATE_HZ = 1000 this is 500 ms.
#define BLINK_DELAY_TICKS 500

/*
 * FreeRTOS task entry point: blinks the Pico W on-board LED forever.
 * Initialises the CYW43 driver itself, so it must run after the scheduler
 * has started.
 */
void blink_task(void *params);

/*
 * FreeRTOS task entry point: starts blink_task, then echoes every character
 * received on stdin back with its letter case swapped.
 */
void main_task(void *params);
