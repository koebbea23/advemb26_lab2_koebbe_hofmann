#include "pico/cyw43_arch.h"

#include "led.h"

void led_put(bool on)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

bool led_get(void)
{
    return cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
}
