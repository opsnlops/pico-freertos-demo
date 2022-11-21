#include <sys/cdefs.h>
#include <pico/printf.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

uint PIN_OUT = 0;
uint slice = 0;
uint channel = 0;

_Noreturn void vBlinkTask() {

    for (;;) {

        printf("on\n");
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(250));

        printf("off\n");
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(250));

    }

}

_Noreturn void vPWMTask() {

    // One millisecond
    int ms = 65465 / 20;

    // Don't yell about this being endless loop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {

        printf("Sweeping...\n");

        // Sweep from 250us to 2500us
        for (int i = ms / 4; i < (ms * 2.5); i += 10) {

            pwm_set_chan_level(slice, channel, i);
            vTaskDelay(pdMS_TO_TICKS(2.5));
        }
    }
#pragma clang diagnostic pop
}


int main() {


    stdio_init_all();

    double divider = 125000000 / (4096 * 50) / 16; // 50Hz

    gpio_set_function(PIN_OUT, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(PIN_OUT);
    channel = pwm_gpio_to_channel(PIN_OUT);

    pwm_set_clkdiv(slice, (float) divider);      // Set the divider to what we came up with
    pwm_set_wrap(slice, 65465);          // Set the wrap time to 20 ms (50Hz)
    pwm_set_enabled(slice, true);


    gpio_init(PICO_DEFAULT_LED_PIN);

    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    xTaskCreate(vBlinkTask, "Blink Task", 128, NULL, 1, NULL);
    xTaskCreate(vPWMTask, "PWM Task", 1024, NULL, 1, NULL);

    // Run free, little tasks!
    vTaskStartScheduler();

    // Remove ourselves from the queue. Byyyyye!
    vTaskDelete(NULL);


}