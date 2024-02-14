#include "Cli.h"

cliSysNotifType g_texts[CLI_MAX_NUM_OF_SYS_NOTIF];    // pointers to texts that should be displayed
static uint8_t g_textsIndx = 0;  // index next available space in g_texts array

void Cli_init(void)
{
    CLI_INFO_0NL("Starting Cli_init()");
}

/**
 * @brief Function that stores pointer to text that is to be displayed
 * @param text text to be displayed
 * @return (void)
 */
void Cli_Write_SysNotif(const char* text)
{
    if (nullptr != text)
    {
        // Calculate the length of the text to determine the size parameter.
        // This assumes string is null-terminated.
        uint8_t size = 0;  // go from 255 to
        while('\0' != text[size])
        {
            size++;
            if (size == 255)
            {
                break;
            }
        }

        if (CLI_MAX_NUM_OF_SYS_NOTIF >= g_textsIndx)
        {
            if (0 != size)
            {
                g_texts[g_textsIndx].textPtr = text;
                g_texts[g_textsIndx].textSize = size;
            }
            else
            {
                const char errorText[] = "Error in Cli.cpp - size is 0\n";
                g_texts[g_textsIndx].textPtr = errorText;
                g_texts[g_textsIndx].textSize = sizeof(errorText) / sizeof(errorText[0]);
            }
            g_textsIndx++;
        }
    }
    else
    {
        const char errorText[] = "Error in Cli.cpp - text pointer is NULL\n";
        g_texts[g_textsIndx].textPtr = errorText;
        g_texts[g_textsIndx].textSize = sizeof(errorText) / sizeof(errorText[0]);
    }
}

cliSysNotifType* Cli_Read_SysNotifArr()
{
    return g_texts;
}

uint8_t Cli_Read_NumOfSysNotif()
{
    uint8_t numOfTxt = g_textsIndx;
    g_textsIndx = 0;
    return numOfTxt;
}