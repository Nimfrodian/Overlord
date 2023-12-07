#include "Cli.h"
#include "RTE.h"


#define UART_BAUD (921600u)
#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024)

#define UART_WRITE(x) uart_write_bytes(UART_NUM, x, strlen(x+1));
#define UART_WRITE_NEWLINE(x) uart_write_bytes(UART_NUM, x, strlen(x)); uart_write_bytes(UART_NUM, "\n", 1);

uint8_t _data[BUF_SIZE];
int _length;
int numCommands = 0;    // number of commands in commands[]
extern cliCmdType commands[];

static void printTime(void)
{
    uint32_t seconds = (esp_timer_get_time() /1000) / 1000; // Convert milliseconds to seconds
    uint32_t minutes = seconds / 60;                        // Convert seconds to minutes
    uint32_t hours = minutes / 60;                          // Convert minutes to hours
    uint32_t days = hours / 24;                             // Convert hours to days
    uint32_t months = days / 30;                            // Approximate days to months
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    days %= 30;  // Assuming each month as 30 days

    char timePrintable[128] = {0};
    int length = 0;
    length += sprintf(timePrintable + sizeof(char) * length, "Uptime ");

    if (months)
    {
        length += sprintf(timePrintable + length, "%d month" , months);
        if ((1 < months) || (0 == months))
        {
            length += sprintf(timePrintable + sizeof(char) * length, "s");
        }
        length += sprintf(timePrintable + sizeof(char) * length, ", ");
    }
    if (days || months)
    {
        length += sprintf(timePrintable + length, "%d day" , days);
        if ((1 < days) || (0 == days))
        {
            length += sprintf(timePrintable + sizeof(char) * length, "s");
        }
        length += sprintf(timePrintable + sizeof(char) * length, ", ");
    }
    {
        length += sprintf(timePrintable + length, "%dh %dm %ds" , hours, minutes, seconds);
    }
    UART_WRITE_NEWLINE(timePrintable)
}



static void help(uint32_t argc, char* argv[])
{
    uart_write_bytes(UART_NUM, "HELP:\n", 7);
    if (0 == argc)
    {
        uart_write_bytes(UART_NUM, "Available commands;\n", 21);
        for (int i = 0; i < numCommands; i++)
        {
            char printable[1024] = {0};
            int length = sprintf(printable, "\t%s", commands[i].commandStr);
            printable[length++] = ' ';
            while (length < 15)
            {
                printable[length++] = '.';
            }
            printable[length++] = ' ';
            length += sprintf(printable + length, "%s\n", commands[i].commandDes);

            uart_write_bytes(UART_NUM, printable, length);
        }
    }
    else if (1 == argc)
    {
        bool cmdFound = false;
        for (int i = 0; i < numCommands; i++)
        {
            if ((strlen(commands[i].commandStr) == strlen(argv[0])) &&                          // length match
                (strncmp(argv[0], commands[i].commandStr, strlen(commands[i].commandStr)) == 0))   // content match
                {
                    cmdFound = true;
                    if (commands[i].detailedDesc != NULL)
                    {
                        uart_write_bytes(UART_NUM, commands[i].detailedDesc, strlen(commands[i].detailedDesc));
                    }
                    else
                    {
                        uart_write_bytes(UART_NUM, "No further details. See 'help' for a basic description", 56);
                    }
                    break;
                }
        }
        if (false == cmdFound)
        {
            char printable[128] = {0};
            int length = sprintf(printable, "Command %s was not found, see 'help' for available commands", argv[0]);
            uart_write_bytes(UART_NUM, printable, length);
        }
    }
    else
    {
        uart_write_bytes(UART_NUM, "Wrong number of arguments. Try 'help' for list of command names, or 'help [command name]' for more on that particular command.\n", 7);
    }
}

static void dio_sts(uint32_t argc, char* argv[])
{
    UART_WRITE_NEWLINE("--- Input status ---\n")
    for (int i = 0; i < NUM_OF_INPUT_INDX; i++)
    {
        char printable[128] = {0};
        int length = sprintf(printable, "In%02d:%d", i, Rte_Dio_read_gpioSt(i));
        if (((i+1) % 8) == 0)
        {
            length += sprintf(printable + length, "\n");
        }
        else
        {
            length += sprintf(printable + length, "   ");
        }

        uart_write_bytes(UART_NUM, printable, length);
    }
    UART_WRITE_NEWLINE("")
}

static void relay_sts(uint32_t argc, char* argv[])
{
    UART_WRITE_NEWLINE("--- Relay status ---\n")
    for (int i = 0; i < (RELAY_MODULE_NUM_OF_RELAY_BOARDS * 8); i++)
    {
        char printable[128] = {0};
        int length = sprintf(printable, "Rly%02d:%d", i, Rte_Relay_read_relaySt(i));
        if (((i+1) % 8) == 0)
        {
            length += sprintf(printable + length, "\n");
        }
        else
        {
            length += sprintf(printable + length, "   ");
        }

        uart_write_bytes(UART_NUM, printable, length);
    }
    UART_WRITE_NEWLINE("")
}

static void set_relay(uint32_t argc, char* argv[])
{
    if (2 == argc)
    {
        int relayIndx = atoi(argv[0]);
        int relayState = atoi(argv[1]);
        bool relaySet = Rte_Relay_write_relaySt(relayIndx, relayState);
        char printable[128] = {0};
        int length = 0;
        if (relaySet)
        {
            length = sprintf(printable, "Relay %d was set to %d", relayIndx, relayState);
        }
        else
        {
            length = sprintf(printable, "Relay %d was not set to %d", relayIndx, relayState);
        }
        uart_write_bytes(UART_NUM, printable, length);
    }
    else
    {
        char printable[128] = {0};
        int length = sprintf(printable, "Invalid number of arguments, expected 2 but got %d", argc);
        uart_write_bytes(UART_NUM, printable, length);
    }
}

static void power_sts(uint32_t argc, char* argv[])
{
    char printable[128] = {0};
    int length = 0;
    if (2 == argc)
    {
        int meterIndx = atoi(argv[0]);
        int meterVar = atoi(argv[1]);
        if (meterIndx >= SDM120M_NUM_OF_MODULES)
        {
            length = sprintf(printable, "Power meter index %d out of range, maximum value is %d\n", meterIndx, SDM120M_NUM_OF_MODULES);
        }
        else if (meterIndx >= SDM120M_NUM_OF_READ)
        {
            length = sprintf(printable, "Variable index %d out of range, maximum value is %d\n", meterVar, SDM120M_NUM_OF_READ);
        }
        else
        {
            float varValue = Sdm120m_read_dataValue(meterIndx, meterVar);
            length = sprintf(printable, "%s of module %d is %f\n", Rte_Sdm120m_read_dataName(meterVar), meterIndx, varValue);
        }
        uart_write_bytes(UART_NUM, printable, length);
    }
    else
    {
        length = sprintf(printable, "Invalid number of arguments, expected 2 but got %d", argc);
        uart_write_bytes(UART_NUM, printable, length);
    }
}

static void power_sts_all(uint32_t argc, char* argv[])
{
    char printable[256] = {0};
    int length = 0;

    if (1 == argc)
    {
        UART_WRITE_NEWLINE("--- Power meter status ---")

        int dataLevel = atoi(argv[0]);
        for (int meterIndx = 0; meterIndx < SDM120M_NUM_OF_MODULES; meterIndx++)
        {
            length += sprintf(printable + length, "\tMeter ID 0x%.2X\n", Rte_Sdm120m_read_moduleId(meterIndx));
            for (int meterVar = 0; meterVar < SDM120M_NUM_OF_READ; meterVar++)
            {
                if (0 == dataLevel)
                {
                    // ignore list
                    if ((SDM120M_APPARENT_POWER_VA == meterVar) ||
                         (SDM120M_REACTIVE_POWER_VAr == meterVar) ||
                         (SDM120M_IMPORT_ACTIVE_ENERGY_kWh == meterVar) ||
                         (SDM120M_EXPORT_ACTIVE_ENERGY_kWh == meterVar) ||
                         (SDM120M_IMPORT_REACTIVE_ENERGY_kVArh == meterVar) ||
                         (SDM120M_EXPORT_REACTIVE_ENERGY_kVArh == meterVar) ||
                         (SDM120M_TOTAL_SYSTEM_POWER_DEMAND_W == meterVar) ||
                         (SDM120M_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND_W == meterVar) ||
                         (SDM120M_IMPORT_SYSTEM_POWER_DEMAND_W == meterVar) ||
                         (SDM120M_MAXIMUM_IMPORT_SYSTEM_POWER_DEMAND_W == meterVar) ||
                         (SDM120M_EXPORT_SYSTEM_POWER_DEMAND_W == meterVar) ||
                         (SDM120M_MAXIMUM_EXPORT_SYSTEM_POWER_DEMAND == meterVar) ||
                         (SDM120M_CURRENT_DEMAND_A == meterVar) ||
                         (SDM120M_MAXIMUM_CURRENT_DEMAND_A == meterVar) ||
                         (SDM120M_TOTAL_REACTIVE_ENERGY_kVArh == meterVar))
                        {
                            continue;
                        }
                }
                float varValue = Sdm120m_read_dataValue(meterIndx, meterVar);
                int32_t remainingLength = length;
                length += sprintf(printable + length, "\t\t%s: ", Rte_Sdm120m_read_dataName(meterVar));
                remainingLength = 30 - (length - remainingLength);
                if (remainingLength < 3) remainingLength = 3;
                while (remainingLength--)
                {
                    length += sprintf(printable + length, ".");
                }
                length += sprintf(printable + length, " %f\n", varValue);
                uart_write_bytes(UART_NUM, printable, length);
                length = 0;
            }
        }
    }
    else
    {
        length += sprintf(printable + length, "Invalid number of arguments, expected 1 but got %d", argc);
        uart_write_bytes(UART_NUM, printable, length);
    }
}

static void dev_sts(uint32_t argc, char* argv[])
{
    UART_WRITE_NEWLINE("------- Device status --------------")
    printTime();
    // print MODBUS status
    // print CAN status
    dio_sts(0,NULL);
    relay_sts(0,NULL);
    char* argv1[] = {"0"};
    power_sts_all(1, argv1);
    UART_WRITE_NEWLINE("------- End of Device status -------")
}

cliCmdType commands[]
{
    {
        .commandStr = "help",
        .commandDes = "prints out available commands",
        .detailedDesc = "use 'help' or 'help [command name]' for a detailed description of what the command does",
        .cmdFunc = help
    },
    {
        .commandStr = "dev_sts",
        .commandDes = "prints out device status",
        .detailedDesc = NULL,
        .cmdFunc = dev_sts
    },
    {
        .commandStr = "dio_sts",
        .commandDes = "prints out DIO status",
        .detailedDesc = NULL,
        .cmdFunc = dio_sts
    },
    {
        .commandStr = "relay_sts",
        .commandDes = "prints out relay status",
        .detailedDesc = NULL,
        .cmdFunc = relay_sts
    },
    {
        .commandStr = "set_relay",
        .commandDes = "relay x,y -> sets [x] relay to [y] state",
        .detailedDesc = NULL,
        .cmdFunc = set_relay
    },
    {
        .commandStr = "power_sts",
        .commandDes = "power_sts x,y -> reads [y] variable from [x] power meter",
        .detailedDesc = "available y indexes are:\n"\
                            "\t 0 READ_VOLTAGE_V,\n"\
                            "\t 1 READ_CURRENT_A,\n"\
                            "\t 2 ACTIVE_POWER_W,\n"\
                            "\t 3 APPARENT_POWER_VA,\n"\
                            "\t 4 REACTIVE_POWER_VAr,\n"\
                            "\t 5 POWER_FACTOR,\n"\
                            "\t 6 FREQUENCY,\n"\
                            "\t 7 IMPORT_ACTIVE_ENERGY_kWh,\n"\
                            "\t 8 EXPORT_ACTIVE_ENERGY_kWh,\n"\
                            "\t 9 IMPORT_REACTIVE_ENERGY_kVArh,\n"\
                            "\t10 EXPORT_REACTIVE_ENERGY_kVArh,\n"\
                            "\t11 TOTAL_SYSTEM_POWER_DEMAND_W,\n"\
                            "\t12 MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND_W,\n"\
                            "\t13 IMPORT_SYSTEM_POWER_DEMAND_W,\n"\
                            "\t14 MAXIMUM_IMPORT_SYSTEM_POWER_DEMAND_W,\n"\
                            "\t15 EXPORT_SYSTEM_POWER_DEMAND_W,\n"\
                            "\t16 MAXIMUM_EXPORT_SYSTEM_POWER_DEMAND,\n"\
                            "\t17 CURRENT_DEMAND_A,\n"\
                            "\t18 MAXIMUM_CURRENT_DEMAND_A,\n"\
                            "\t19 TOTAL_ACTIVE_ENERGY_kWh,\n"\
                            "\t20 TOTAL_REACTIVE_ENERGY_kVArh\n"\
                        "available x indexes are from 0 up to 20",
        .cmdFunc = power_sts
    },
    {
        .commandStr = "power_sts_all",
        .commandDes = "power_sts_all x -> prints all power variables; [x=0] just the basics, [x>0] everything",
        .detailedDesc = NULL,
        .cmdFunc = power_sts_all
    },
};

void Rte_Cli_init(void)
{
    // Configure UART 0
    {
        uart_config_t uart0_config = {
                .baud_rate = UART_BAUD,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 122,
                .source_clk = UART_SCLK_APB,
        };
        // Configure UART 0 pins
        uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        // Set UART 0 parameters
        uart_param_config(UART_NUM, &uart0_config);
        // Install UART 0 driver
        uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);
        // Set RS485 half duplex mode
        uart_set_mode(UART_NUM, UART_MODE_RS485_HALF_DUPLEX);

        uart_flush(UART_NUM);
    }

    numCommands = sizeof(commands) / sizeof(commands[0]);
}

void Rte_Cli_run(void)
{
    _length += uart_read_bytes(UART_NUM, &_data[_length], BUF_SIZE, 100 / portTICK_RATE_MS);
    if (_length > 0) {
        if ((_length == 1) && (_data[_length - 1] == '\n')) // only 'enter' was sent, ignore
        {
            _length = 0;
        }
        else if (_data[_length - 1] == '\n') // if line feed then it is end of command
        {
            //char printable[100] = {0};
            //int length = sprintf(printable, "length:%d\n", _length);
            //uart_write_bytes(UART_NUM, printable, length);

            // Null-terminate the data
            _data[_length] = '\0';

            // extract command
            char extractedCmd[100] = {0};
            char* argv[10] = {0};   // pointers to arguments
            char args[2048] = {0};   // actual arguments
            argv[0] = &args[0];
            uint32_t argc = 0;      // number of arguments
            uint32_t endOfCmdPos = 0;   // index at which the command is over and arguments, if any, begin
            for (int i = 0; i < _length; i++)
            {
                // if space is found, or end of command then terminate
                if ('\r' == _data[i])
                {
                    // ignore
                }
                else if ('\n' == _data[i])
                {
                    break;  // break out
                }
                else if (' ' == _data[i])
                {
                    //char printable[1000] = {0};
                    //int length = sprintf(printable, "Command name:%s, ended at %d\n", extractedCmd, endOfCmdPos);
                    //uart_write_bytes(UART_NUM, printable, length);

                    // extract arguments
                    // dev_sts 10,32,100 -> 3, [10,32,100]
                    int argPos = 0;         // temp var to keep track of position within actual arguments
                    for (int i = endOfCmdPos + 1; i < _length; i++)
                    {
                        if (10 <= argc)
                        {
                            break;  // too many arguments
                        }
                        else if ('\r' == _data[i])
                        {
                            // ignore
                        }
                        else if ((',' == _data[i]) || ('\n' == _data[i]))
                        {
                            //char printable[100] = {0};
                            //int length = sprintf(printable, "\tExtracted:%s\n", argv[argc]);
                            //uart_write_bytes(UART_NUM, printable, length);
                            *(argv[argc] + sizeof(char) * argPos) = '\0';   // terminate the argument
                            argc++;
                            argv[argc] = argv[argc - 1] + (argPos + 1) * sizeof(char);
                            argPos = 0;
                        }
                        else
                        {
                            // take location of argc argument, go for argPos further along and write there _data[i]
                            *(argv[argc] + sizeof(char) * argPos) = _data[i];
                            argPos++;
                        }
                    }
                    break;
                }
                else
                {
                    extractedCmd[i] = _data[i];
                    endOfCmdPos++;
                }
            }


            // Check each command and add arguments
            for (int i = 0; i < numCommands; i++)
            {
                //char printable[1000] = {0};
                //int length = sprintf(printable, "\t\tComparing:%s to %s\n", commands[i].commandStr, extractedCmd);
                //uart_write_bytes(UART_NUM, printable, length);
                if ((strlen(commands[i].commandStr) == strlen(extractedCmd)) &&                          // length match
                (strncmp(extractedCmd, commands[i].commandStr, strlen(commands[i].commandStr)) == 0))   // content match
                {
                    // Execute the corresponding function
                    commands[i].cmdFunc(argc, argv);

                    break;
                }


                if ((numCommands - 1) == i)
                {
                    char printable[1024] = {0};
                    int length = sprintf(printable, "Command '%s' is not recognized. Send 'help' for a list of commands\n", extractedCmd);
                    uart_write_bytes(UART_NUM, printable, length);
                }
            }

            // clear variables for new command
            uart_flush(UART_NUM_1);
            _length = 0;
        }
    }
}

