#include "LogAnalyzer.h"
#include "getopt.h"
#include "string.h"

typedef struct _LogLevelMap_
{
    uint8_t text[7];
    LogLevel level;
}LogLevelMap;

LogLevelMap level_map[] = {
    {"VERBOSE", LEVEL_VERBOSE}, {"0",       LEVEL_VERBOSE}, {"Verbose", LEVEL_VERBOSE}, {"verbose", LEVEL_VERBOSE}, 
    {"INFO", LEVEL_INFO},       {"1",    LEVEL_INFO},       {"Info", LEVEL_INFO},       {"info", LEVEL_INFO}, 
    {"WARN", LEVEL_WARN},       {"2",       LEVEL_WARN},    {"Warn", LEVEL_WARN},       {"warn", LEVEL_WARN}, 
    {"ERROR", LEVEL_ERROR},     {"3",       LEVEL_ERROR},   {"Error", LEVEL_ERROR},     {"error", LEVEL_ERROR},
};

int main(int argc, char *argv[])
{
    int opt;
    char *findKeyword = NULL;
    char *logPath = NULL;
    char *level = NULL;

    int pathSetFlag = 0;
    
    static struct option long_option[] = {
        {"find",    required_argument, NULL, 'f'},
        {"path",    required_argument, NULL, 'p'},
        {"help",    no_argument,       NULL, 'h'},
        {"select",  no_argument,       NULL, 's'},
        {"all",     no_argument,       NULL, 'a'},
        {0, 0, 0, 0}
    };

    // init
    u8InitLogAnalyzer();

    // printf("Usage: %s -p <logfile> -f <keyword> -s <loglevel> -a -h\n", argv[0]);

    while((opt = getopt_long(argc, argv, "f:p:s:ah", long_option, NULL)) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                logPath = optarg;
                pathSetFlag = 1;
                u8InputLogPath((uint8_t*)logPath, strlen(logPath));
                break;
            }
            case 'f':
            {
                if(!pathSetFlag)
                {
                    printf("Please use  -f/-path input the log file path first.\n");
                    return 1;
                }
                findKeyword = optarg;
                u8KeyWordSearch((uint8_t*)findKeyword, strlen(findKeyword));
                break;
            }
            case 's':
            {
                if(!pathSetFlag)
                {
                    printf("Please use  -f/-path input the log file path first.\n");
                    return 1;
                }
                LogLevel elevel = LEVEL_INVALID;
                level = optarg;
                int i = 0;
                for(i = 0; i < (sizeof(level_map)/sizeof(LogLevelMap)); i++)
                {
                    if(0 == strcmp(level, level_map[i].text))
                    {
                        elevel = level_map[i].level;
                        break;
                    }
                }
                if(i >= (sizeof(level_map)/sizeof(LogLevelMap)))
                {
                    printf("Please input valid log level, like verbose/info/warn/error \n");
                    return 1;
                }
                else
                {
                    uint32_t tmp = 0;
                    tmp = u32CounterLogLinesSelect(elevel);
                    printf("\nLog level: %s, lines: %d\n", level, tmp);
                }
                break;
            }
            case 'a':
            {
                if(!pathSetFlag)
                {
                    printf("Please use  -f/-path input the log file path first.\n");
                    return 1;
                }
                // log lines counter
                uint32_t total = 0;
                total = u32CounterLogLinesTotal();
                printf("log file total lines: %d\n", total);
                break;
            }
            case 'h':
            {
                printf("Usage: %s -p <logfile> -f <keyword> -s <loglevel> -a\n", argv[0]);
                break;
            }
            case '?':
            {
                printf("Usage: %s -p <logfile> -f <keyword> -s <loglevel> -a -h\n", argv[0]);
                return 1;
            }
            default:
            {
                printf("default Usage: %s -p <logfile> -f <keyword> -s <loglevel> -a -h\n", argv[0]);
                break;
            }
        }
    }

    u8DeinitLogAnalyzer();

    return 0;
}

