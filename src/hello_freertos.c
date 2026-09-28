/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Overview
 * --------
 * A small FreeRTOS demo for the Pico W with two tasks (see src/tasks.c):
 *
 *   main_task  - a USB-serial "case swapper". Every character received on
 *                stdin is echoed back with its letter case inverted.
 *   blink_task - blinks the on-board LED every 500 ms, skipping one toggle
 *                every 11 iterations (one 1 s gap every 5.5 s).
 *
 * This file only holds main(), which runs once on reset, creates main_task,
 * and hands the CPU to the FreeRTOS scheduler.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"

#include "tasks.h"

int main( void )
{
    // Bring up USB CDC serial so getchar()/putchar() work.
    stdio_init_all();
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);
    // Start FreeRTOS. From here on the scheduler owns the CPU. This call
    // only returns if there was not enough heap to start.
    vTaskStartScheduler();
    return 0;
}
