#ifndef CLI_H
#define CLI_H

#include <inttypes.h>
#include "driver/uart.h"
#include "string.h"

typedef struct
{
    const char* commandStr;                               // Command name
    const char* commandDes;                               // Command description
    const char* detailedDesc;
    void (*cmdFunc)(uint32_t argc, const char* argv[]);   // function to execute when command string is received
} cliCmdType;

extern cliCmdType commands[];

#endif