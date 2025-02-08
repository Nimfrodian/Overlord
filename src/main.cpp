#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "freertos/timers.h"
#include "RTE.h"

// Firmware version 1.00.00

static void tasks_10ms_callback(TimerHandle_t xTimer);

extern "C" void app_main(void)
{
    CLI_INFO_0NL("Starting app_main()");

    Rte_Cli_init();

    ComModbus_init();   // initialize Modbus
    ComCan_init();      // initialize CAN
    BSW_Dio_init();    // initialize GPIO control

    Rte_RelayControl_init();    // initialize Relay control
    Rte_Sdm120m_init();         // initialize SDM120M power meter module control


    // Create a timer to run the tasks_10ms task periodically, the "main" task
    TimerHandle_t tasks_10ms_timer = xTimerCreate(
            "tasks_10ms_timer",             // Timer name
            pdMS_TO_TICKS(10),              // Timer period (in ticks)
            pdTRUE,                         // Auto-reload timer
            NULL,                           // Timer ID
            tasks_10ms_callback);           // Timer callback function

    // Start the timer
    xTimerStart(tasks_10ms_timer, 0);


    gpio_reset_pin(GPIO_NUM_15);    // TODO
    gpio_reset_pin(GPIO_NUM_0); // TODO
    gpio_reset_pin(GPIO_NUM_2); // TODO
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    uint8_t counter = 0;
    while (1)
    {
        gpio_set_level(GPIO_NUM_15, counter & 0x01);
        gpio_set_level(GPIO_NUM_0, counter & 0x02);
        gpio_set_level(GPIO_NUM_2, counter & 0x04);
        counter++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void tasks_10ms_callback(TimerHandle_t xTimer)
{
    // run the SDM120M power meter control
    Rte_Sdm120m_runnable_10ms();

    // run the relay module logic
    Rte_RelayControl_runnable_10ms();

    // run GPIO control
    Rte_Dio_runnable_10ms();

    // run CLI runnable
    Rte_Cli_run();

    static uint8_t cntr = 0;
    if (cntr == 0)
    {
        CLI_INFO_0NL("Counter rolled over");
    }
    cntr++;

}
