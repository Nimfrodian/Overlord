#include "rtdb.h"
#include "mdll.h"
#include "crcm.h"

#if defined(PLATFORM_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t db_mutex;

static tB rtdp_lock(void)
{
    tB taken = xSemaphoreTake(db_mutex, pdMS_TO_TICKS(50)) == pdTRUE;
    return taken;
}

static void rtdp_unlock(void)
{
    xSemaphoreGive(db_mutex);
}


void rtdp_init(void)
{
    tRTDB_INITDATA_STR RtdbCfg =
    {
        .rtdb_lock_callback = rtdp_lock,
        .rtdb_unlock_callback = rtdp_unlock,
        .CRC32_callback = crcm_CRC32,
    };
    rtdb_init(&RtdbCfg);

    db_mutex = xSemaphoreCreateMutex();
    canm_rtdb_init();
    dimh_rtdb_init();
    diom_rtdb_init();
    mbcm_rtdb_init();
}


#elif defined(PLATFORM_LOCALHOST)



#endif