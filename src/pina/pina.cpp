#include "pina.h"
#include "esp_attr.h"

static bool pina_s_moduleInit_tB = false;
static uint32_t pina_nr_moduleId_U32 = 0;

void pina_init(tPINA_INITDATA_STR* PinaCfg)
{
    if (true == pina_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, pina_nr_moduleId_U32, 0, PINA_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == PinaCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, pina_nr_moduleId_U32, 0, PINA_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        pina_nr_moduleId_U32 = PinaCfg->nr_moduleId_U32;
        pina_s_moduleInit_tB = true;
    }
}

void pina_setGpioLevel(PINA_nr_GPIO_NUM_E GpioNum, bool Value)
{
    gpio_set_level((gpio_num_t) GpioNum, Value);
}

void pina_setGpioAsOutput(PINA_nr_GPIO_NUM_E GpioNum)
{
    gpio_reset_pin((gpio_num_t) GpioNum);
    gpio_intr_disable((gpio_num_t) GpioNum);
    gpio_pulldown_dis((gpio_num_t) GpioNum);
    gpio_pullup_dis((gpio_num_t) GpioNum);
    gpio_set_direction((gpio_num_t) GpioNum, GPIO_MODE_OUTPUT);
}

void pina_setGpioAsInput(PINA_nr_GPIO_NUM_E GpioNum)
{
    gpio_pulldown_en((gpio_num_t) GpioNum);
    gpio_set_direction((gpio_num_t) GpioNum, GPIO_MODE_INPUT);
    gpio_set_intr_type((gpio_num_t) GpioNum, GPIO_INTR_NEGEDGE);
    gpio_intr_enable((gpio_num_t) GpioNum);
}

void pina_setInterruptService(PINA_nr_GPIO_NUM_E GpioNum, void (*func)(void*arg))
{
    gpio_install_isr_service(0); ///< install the ISR service with default configuration
    gpio_isr_handler_add((gpio_num_t) GpioNum, func, (void*) GpioNum); ///< add the custom ISR handler

}