#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "freertos/timers.h"
#include <stdio.h>
#include "RTE.h"

// Firmware version 1.00.00

static void tasks_10ms_callback(TimerHandle_t xTimer);

void UART_0_transceive(void* param)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100u));
    }


    // delete task if illegal state was reached
    vTaskDelete( NULL );
}

extern "C" void app_main(void)
{
    ComModbus_init();   // initialize Modbus
    ComCan_init();      // initialize CAN
    BSW_Dio_init();    // initialize GPIO control

    Rte_RelayControl_init();    // initialize Relay control
    Rte_Sdm120m_init();         // initialize SDM120M power meter module control

    // Configure UART 0
    {
        uart_config_t uart0_config = {
                .baud_rate = 115200,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 122,
                .source_clk = UART_SCLK_APB,
        };
        // Configure UART 0 pins
        uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        // Set UART 0 parameters
        uart_param_config(UART_NUM_0, &uart0_config);
        // Install UART 0 driver
        uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
        // Set RS485 half duplex mode
        uart_set_mode(UART_NUM_0, UART_MODE_RS485_HALF_DUPLEX);
        // create transceive task
        xTaskCreatePinnedToCore(
        UART_0_transceive,    // Function that should be called
        "UART_0_transceive",   // Name of the task (for debugging)
        8192,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL,            // Task handle
        1                // run on core 1
        );
    }

    // Create a timer to run the tasks_10ms task periodically, the "main" task
    TimerHandle_t tasks_10ms_timer = xTimerCreate(
            "tasks_10ms_timer",             // Timer name
            pdMS_TO_TICKS(10),              // Timer period (in ticks)
            pdTRUE,                         // Auto-reload timer
            NULL,                           // Timer ID
            tasks_10ms_callback             // Timer callback function
    );
    // Start the timer
    xTimerStart(tasks_10ms_timer, 0);

    while (1)
    {
        vTaskSuspend(NULL);
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
}