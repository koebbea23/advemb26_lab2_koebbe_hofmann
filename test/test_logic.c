/*
 * Unit tests for the hardware-independent logic in src/blink.c and
 * src/case_swap.c. Nothing here touches the CYW43 or FreeRTOS, so these
 * tests run on the Pico, in Renode, and in CI.
 */
#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <unity.h>
#include "unity_config.h"
#include "blink.h"
#include "case_swap.h"

/* ---- Fake LED: records every value blink_step() writes. ---- */
#define FAKE_LED_MAX 64
static bool fake_led_writes[FAKE_LED_MAX];
static int fake_led_write_count;

static void fake_led_put(bool on)
{
    if (fake_led_write_count < FAKE_LED_MAX)
        fake_led_writes[fake_led_write_count] = on;
    fake_led_write_count++;
}

void setUp(void)
{
    fake_led_write_count = 0;
}

void tearDown(void) {}

/* ---- switch_case ---- */

void test_switch_case_lower_to_upper(void)
{
    for (char c = 'a'; c <= 'z'; c++)
        TEST_ASSERT_EQUAL_CHAR(c - 'a' + 'A', switch_case(c));
}

void test_switch_case_upper_to_lower(void)
{
    for (char c = 'A'; c <= 'Z'; c++)
        TEST_ASSERT_EQUAL_CHAR(c - 'A' + 'a', switch_case(c));
}

void test_switch_case_boundaries_unchanged(void)
{
    // The characters right next to each letter range must not be changed.
    TEST_ASSERT_EQUAL_CHAR('@', switch_case('@'));  // 'A' - 1
    TEST_ASSERT_EQUAL_CHAR('[', switch_case('['));  // 'Z' + 1
    TEST_ASSERT_EQUAL_CHAR('`', switch_case('`'));  // 'a' - 1
    TEST_ASSERT_EQUAL_CHAR('{', switch_case('{'));  // 'z' + 1
}

void test_switch_case_non_letters_unchanged(void)
{
    // Every byte value that is not an ASCII letter passes through unchanged,
    // including control codes and bytes >= 0x80.
    for (int i = 0; i < 256; i++) {
        char c = (char)i;
        bool is_letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        if (!is_letter)
            TEST_ASSERT_EQUAL_CHAR(c, switch_case(c));
    }
}

void test_switch_case_round_trip(void)
{
    for (int i = 0; i < 256; i++)
        TEST_ASSERT_EQUAL_CHAR((char)i, switch_case(switch_case((char)i)));
}

/* ---- blink_next_state ---- */

void test_blink_first_iteration_does_not_toggle(void)
{
    int count = 0;
    TEST_ASSERT_FALSE(blink_next_state(false, &count));
    count = 0;
    TEST_ASSERT_TRUE(blink_next_state(true, &count));
}

void test_blink_increments_count(void)
{
    int count = 41;
    blink_next_state(false, &count);
    TEST_ASSERT_EQUAL_INT(42, count);
}

void test_blink_toggles_when_count_not_multiple_of_period(void)
{
    for (int start = 1; start < BLINK_PERIOD; start++) {
        int count = start;
        TEST_ASSERT_TRUE(blink_next_state(false, &count));
        count = start;
        TEST_ASSERT_FALSE(blink_next_state(true, &count));
    }
}

void test_blink_holds_when_count_multiple_of_period(void)
{
    int multiples[] = {0, BLINK_PERIOD, 2 * BLINK_PERIOD, 10 * BLINK_PERIOD};
    for (unsigned i = 0; i < sizeof multiples / sizeof multiples[0]; i++) {
        int count = multiples[i];
        TEST_ASSERT_FALSE(blink_next_state(false, &count));
        count = multiples[i];
        TEST_ASSERT_TRUE(blink_next_state(true, &count));
    }
}

/* ---- blink_step (with the injected fake LED) ---- */

void test_blink_step_writes_current_state_once(void)
{
    int count = 1;
    bool next = blink_step(true, &count, fake_led_put);
    TEST_ASSERT_EQUAL_INT(1, fake_led_write_count);
    TEST_ASSERT_TRUE(fake_led_writes[0]);   // writes `on`, not the next state
    TEST_ASSERT_FALSE(next);
    TEST_ASSERT_EQUAL_INT(2, count);
}

void test_blink_step_matches_original_sequence(void)
{
    // Characterization test: LED values written by the original,
    // pre-refactor loop in hello_freertos.c, starting from reset
    // (count = 0, on = false). Covers three full periods.
    static const bool expected[] = {
        0,0,1,0,1,0,1,0,1,0,1,
        0,0,1,0,1,0,1,0,1,0,1,
        0,0,1,0,1,0,1,0,1,0,1,
    };
    const int n = sizeof expected / sizeof expected[0];
    int count = 0;
    bool on = false;
    for (int i = 0; i < n; i++)
        on = blink_step(on, &count, fake_led_put);

    TEST_ASSERT_EQUAL_INT(n, fake_led_write_count);
    for (int i = 0; i < n; i++)
        TEST_ASSERT_EQUAL_MESSAGE(expected[i], fake_led_writes[i],
                                  "LED sequence differs from original code");
}

int main(void)
{
    stdio_init_all();
    // Repeat forever instead of exiting: on WSL/usbipd the serial port can
    // take several seconds to reach the host after reboot, so a single run
    // is easily missed. Reflash with `picotool load -f`.
    while (true) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_switch_case_lower_to_upper);
        RUN_TEST(test_switch_case_upper_to_lower);
        RUN_TEST(test_switch_case_boundaries_unchanged);
        RUN_TEST(test_switch_case_non_letters_unchanged);
        RUN_TEST(test_switch_case_round_trip);
        RUN_TEST(test_blink_first_iteration_does_not_toggle);
        RUN_TEST(test_blink_increments_count);
        RUN_TEST(test_blink_toggles_when_count_not_multiple_of_period);
        RUN_TEST(test_blink_holds_when_count_multiple_of_period);
        RUN_TEST(test_blink_step_writes_current_state_once);
        RUN_TEST(test_blink_step_matches_original_sequence);
        UNITY_END();
    }
}
