#ifndef CLI_H
#define CLI_H

#include "driver/uart.h"
#include "string.h"

typedef struct
{
    char* commandStr;                               // Command name
    char* commandDes;                               // Command description
    char* detailedDesc;
    void (*cmdFunc)(uint32_t argc, char* argv[]);   // function to execute when command string is received
} cliCmdType;

extern cliCmdType commands[];

#endif