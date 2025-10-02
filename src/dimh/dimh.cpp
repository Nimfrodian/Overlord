#include "dimh.h"
#include "rtdb.h"
#include "mdll.h"
#include "nvsm.h"

static tB dimh_s_moduleInit_tB = false;

static tU32 dimh_s_prevGpioStates = 0;

void dimh_init(tDIMH_INITDATA_STR* dimhCfg)
{
    if (true == dimh_s_moduleInit_tB)
    {
        errh_reportError(ERRH_NOTIF, MODULE_DIMH, 0, DIMH_API_INIT_U32, ERRH_MODULE_ALREADY_INIT);
    }
    else if (NULL == dimhCfg)
    {
        errh_reportError(ERRH_ERROR_CRITICAL, MODULE_DIMH, 0, DIMH_API_INIT_U32, ERRH_POINTER_IS_NULL);
    }
    else
    {
        // TODO: checks config in NVS and updates RTDB accordingly
        dimh_s_prevGpioStates = rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32);
        dimh_s_moduleInit_tB = true;
    }
}

void dimh_run_5ms(void)
{
    // monitors GPIO states and invert requests and prepares RTDB data accordingly
    tU32 dimh_s_currGpioStates = rtdb_read_tU32S(RTDB_DIOM_X_INPUTSTATES_U32);

    for (tU32 i = 0; i < 32; i++)
    {
        tB prevState = (dimh_s_prevGpioStates >> i) & (0x01);
        tB currState = (dimh_s_currGpioStates >> i) & (0x01);
        if (prevState != currState)
        {
            tU16 newDimVal_U16 = 0;
            if (rtdb_read_tU16S((tU16SEnumT) (RTDB_DIMH_X_TXDIMMERVAL_AU16_0 + i)) >
                rtdb_read_tU16S((tU16SEnumT) (RTDB_DIMH_X_DIMMERTRSHLD_AU16_0 + i)))
            {
                newDimVal_U16 = rtdb_read_tU16S((tU16SEnumT) (RTDB_DIMH_X_DIMMERTRSHLD_AU16_0 + i));
            }
            else
            {
                newDimVal_U16 = rtdb_read_tU16S((tU16SEnumT) (RTDB_DIMH_X_DIMMERON_AU16_0 + i));
            }
            rtdb_write_tU16S((tU16SEnumT) (RTDB_DIMH_X_TXDIMMERVAL_AU16_0 + i), newDimVal_U16);

            rtdb_write_tBS((tBSEnumT) (RTDB_DIMH_S_TXDIMMERRDYFLAG_AB_0 + i), 1);
        }
    }
    dimh_s_prevGpioStates = dimh_s_currGpioStates;
}