#include "Cli.h"


#define UART_BAUD (115200u)
#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024)

#define UART_WRITE(x) uart_write_bytes(UART_NUM, x, strlen(x+1));
#define UART_WRITE_NEWLINE(x) uart_write_bytes(UART_NUM, x, strlen(x)); uart_write_bytes(UART_NUM, "\n", 1);

uint8_t _data[BUF_SIZE];
int _length;
int numCommands = 0;
extern cliCmdType commands[];

static void dev_sts(uint32_t argc, char* argv[])
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

    UART_WRITE_NEWLINE("------- Device status -------")

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
        length += sprintf(printable + sizeof(char) * length, "%s\n", commands[i].commandDes);

        uart_write_bytes(UART_NUM, printable, length);
    }
}

cliCmdType commands[]
{
    {
        .commandStr = "help",
        .commandDes = "prints out available commands",
        .cmdFunc = help
    },
    {
        .commandStr = "dev_sts",
        .commandDes = "prints out device status",
        .cmdFunc = dev_sts
    },
    // print DIO status
    // print RELAY status
    // print POWER METER status
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
                    for (int i = endOfCmdPos; i < _length; i++)
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
                if (strncmp(extractedCmd, commands[i].commandStr, strlen(commands[i].commandStr)) == 0) {
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
            _length = 0;
        }
    }
}

