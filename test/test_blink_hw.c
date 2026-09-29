/*
 * Hardware integration test: blink_step() driving the real Pico W LED
 * through the CYW43. Renode does not model the CYW43 radio, so this test
 * only runs on a real Pico W. The logic itself is covered by test_logic.c.
 */
#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <unity.h>
#include "unity_config.h"
#include "blink.h"
#include "led.h"

void setUp(void)
{
    led_put(false);
}

void tearDown(void)
{
    led_put(false);
}

void test_led_put_and_get(void)
{
    led_put(true);
    TEST_ASSERT_TRUE(led_get());
    led_put(false);
    TEST_ASSERT_FALSE(led_get());
}

void test_blink_step_drives_led(void)
{
    // Run three full periods from reset. After every step the physical LED
    // must hold the state that was passed in, and the next state must match
    // the pure logic.
    int count = 0;
    bool on = false;
    for (int i = 0; i < 3 * BLINK_PERIOD; i++) {
        int logic_count = count;
        bool expected_next = blink_next_state(on, &logic_count);

        bool next = blink_step(on, &count, led_put);

        TEST_ASSERT_EQUAL_MESSAGE(on, led_get(), "LED should show the current state");
        TEST_ASSERT_EQUAL_MESSAGE(expected_next, next, "Next state should match blink_next_state");
        TEST_ASSERT_EQUAL_INT(logic_count, count);
        on = next;
    }
}

int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    // Repeat forever instead of exiting: on WSL/usbipd the serial port can
    // take several seconds to reach the host after reboot, so a single run
    // is easily missed. Reflash with `picotool load -f`.
    while (true) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_led_put_and_get);
        RUN_TEST(test_blink_step_drives_led);
        UNITY_END();
    }
}
