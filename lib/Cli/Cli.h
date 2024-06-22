#ifndef CLI_H
#define CLI_H

#include <inttypes.h>
#include "string.h"

#define CLI_MAX_NUM_OF_SYS_NOTIF (32u)
#define CLI_INFO_0(x) (Cli_Write_SysNotif(x))
#define CLI_INFO_0NL(x) (Cli_Write_SysNotif(x "\n"))

#define CLI_INFO_1(x) (Cli_Write_SysNotif("\t" x))
#define CLI_INFO_1NL(x) (Cli_Write_SysNotif("\t" x "\n"))

#define CLI_INFO_2(x) (Cli_Write_SysNotif("\t\t" x))
#define CLI_INFO_2NL(x) (Cli_Write_SysNotif("\t\t" x "\n"))


typedef struct
{
    const char* textPtr;
    uint8_t textSize;
} cliSysNotifType;


typedef struct
{
    const char* commandStr;                               // Command name
    const char* commandDes;                               // Command description
    const char* detailedDesc;
    void (*cmdFunc)(uint32_t argc, const char* argv[]);   // function to execute when command string is received
} cliCmdType;

extern cliCmdType commands[];

void Cli_init(void);

/**
 * @brief Function to write to console
 * @param text text to print.
 * @return (void)
 */
void Cli_Write_SysNotif(const char* text);


/**
 * @brief Function returns pointer to first text to be displayed. Can be NULL, and is always unformatted.
 * @param void
 * @return pointer to cliSysNotifType
 */
cliSysNotifType* Cli_Read_SysNotifArr(void);

/**
 * @brief Function returns the number of notification messages to be printed
 * @param void
 * @return number of messages to be printed
 */
uint8_t Cli_Read_NumOfSysNotif(void);

#endif