#include "sera.h"

static bool sera_s_moduleInit_tB = false;
static uint32_t sera_nr_moduleId_U32 = 0;

void sera_init(tSERA_INITDATA_STR* SeraCfg)
{
    if (true == sera_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, sera_nr_moduleId_U32, 0, SERA_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == SeraCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, sera_nr_moduleId_U32, 0, SERA_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        ///< Initialize UART0
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_APB,
        };
        uart_param_config(UART_NUM_0, &uart_config);
        uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);

        sera_nr_moduleId_U32 = SeraCfg->nr_moduleId_U32;
        sera_s_moduleInit_tB = true;
    }
}

uint32_t sera_print(const char* Text, ...)
{
    uint32_t bytesWritten = 0;

    if (false == sera_s_moduleInit_tB)
    {
        errh_reportError(ERRH_WARNING, sera_nr_moduleId_U32, 0, SERA_API_WRITE_U32, ERRH_MODULE_NOT_INIT);
    }
    else if (NULL == Text)
    {
        errh_reportError(ERRH_WARNING, sera_nr_moduleId_U32, 0, SERA_API_WRITE_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        char textBuffer[1024] = {0};

        // Initialize a variable argument list
        va_list args;
        va_start(args, Text);

        // Format the text with the variable arguments
        vsprintf(textBuffer, Text, args);

        // Clean up the variable argument list
        va_end(args);

        #if 0
        // Calculate the length of the formatted string
        uint32_t i_U32 = 0;
        while (textBuffer[i_U32] != '\0' && i_U32 < 1024)
        {
            i_U32++;
        }

        // Write the formatted text to the UART
        bytesWritten = uart_write_bytes(UART_NUM_0, textBuffer, i_U32);
        #endif
        bytesWritten = printf(textBuffer);
    }

    return bytesWritten;
}

uint32_t sera_read(char* Buffer, uint32_t BuffLength)
{
    uint32_t bytesRead = 0;
    if (false == sera_s_moduleInit_tB)
    {
        errh_reportError(ERRH_WARNING, sera_nr_moduleId_U32, 0, SERA_API_READ_U32, ERRH_MODULE_NOT_INIT);
    }
    else if (NULL == Buffer)
    {
        errh_reportError(ERRH_WARNING, sera_nr_moduleId_U32, 0, SERA_API_READ_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        bytesRead = uart_read_bytes(UART_NUM_0, Buffer, BuffLength, 0);
    }

    return bytesRead;
}