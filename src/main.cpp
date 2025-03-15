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
    tSERA_INITDATA_STR SeraCfg =
    {
        .nr_moduleId_U32 = MODULE_SERA,
    };
    sera_init(&SeraCfg);
    sera_print("Sera module initialized\n");


    ComModbus_init();   // initialize Modbus
    sera_print("ComModbus module initialized\n");
    ComCan_init();      // initialize CAN
    sera_print("ComCan module initialized\n");
    BSW_Dio_init();    // initialize GPIO control
    sera_print("BSW_Dio module initialized\n");

    Rte_RelayControl_init();    // initialize Relay control
    sera_print("RelayControl module initialized\n");
    Rte_Sdm120m_init();         // initialize SDM120M power meter module control
    sera_print("Sdm120m module initialized\n");


    // Create a timer to run the tasks_10ms task periodically, the "main" task
    TimerHandle_t tasks_10ms_timer = xTimerCreate(
            "tasks_10ms_timer",             // Timer name
            pdMS_TO_TICKS(10),              // Timer period (in ticks)
            pdTRUE,                         // Auto-reload timer
            NULL,                           // Timer ID
            tasks_10ms_callback);           // Timer callback function

    gpio_reset_pin(GPIO_NUM_13);
    gpio_reset_pin(GPIO_NUM_14);
    gpio_reset_pin(GPIO_NUM_21);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);

    sera_print("Setting init complete LED ON\n");
    gpio_set_level(GPIO_NUM_13, 1);

    sera_print("Starting 10ms timer task...");
    // Start the timer
    xTimerStart(tasks_10ms_timer, 0);
    sera_print(" OK!\n");
}

static void tasks_10ms_callback(TimerHandle_t xTimer)
{
    // run the SDM120M power meter control
    Rte_Sdm120m_runnable_10ms();

    // run the relay module logic
    Rte_RelayControl_runnable_10ms();

    // run GPIO control
    Rte_Dio_runnable_10ms();
}
