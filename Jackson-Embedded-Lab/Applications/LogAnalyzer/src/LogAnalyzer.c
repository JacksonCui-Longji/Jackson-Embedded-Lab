#include "LogAnalyzer.h"
#include "CommonType.h"
#include "string.h"

/* internal macro */
#define MAX_CASE_FLOD_TABLE 0x100u
#define LOG_CACHE_LENGTH 0xffu

/* Type define */
typedef struct _LogFileInfo_{
    FILE *LogFP;
    uint8_t LogPath[LOG_MAX_INPUT_LENGTH];
    size_t LogPathLen;
    uint32_t LogLinesTotal;
}LogFileInfo;

/* static Gloable variables */
static LogFileInfo File;
static uint8_t CaseFlodTable[MAX_CASE_FLOD_TABLE];

/* static function declaration */
static void vdKeyWordFileSearch(LogSearchResult *ret);
static LogSearchResult vdInsensitiveSearch(const uint8_t *inputLine, size_t inputLen, const uint8_t *keywords, size_t keyLen);

/* provided function interface */
uint8_t u8InitLogAnalyzer(void)
{
    memset(&File, 0x00, sizeof(LogFileInfo));
    File.LogFP = NULL;
    // init case flod table
    for (uint16_t i = 0; i < MAX_CASE_FLOD_TABLE; i++)
    {
        if ((i >= 'A') && (i <= 'Z'))
        {
            CaseFlodTable[i] = i +  ('a' - 'A');
        }
        else
        {
            CaseFlodTable[i] = i;
        }
    }
    return RET_OK;
}

uint8_t u8DeinitLogAnalyzer(void)
{
    memset(&File, 0x00, sizeof(LogFileInfo));
    if(NULL != File.LogFP)
    {
        fclose(File.LogFP);
        File.LogFP = NULL;
    }
    return RET_OK;
}

uint8_t u8InputLogPath(uint8_t *LogPath, size_t path_len)
{
    if (NULL == LogPath)
    {
        printf("Please input valid log path!\n");
        return RET_NG;
    }
    
    if(0 >= path_len)
    {
        printf("Please input valid log path!\n");
        return RET_NG;
    }

    memset(File.LogPath, 0x00, LOG_MAX_INPUT_LENGTH);
    if (path_len > LOG_MAX_INPUT_LENGTH)
    {
        printf("input size is too long to cache!\n");
        File.LogPathLen = LOG_MAX_INPUT_LENGTH-1;
    }
    else
    {
        File.LogPathLen = path_len;
    }
    
    memcpy(File.LogPath, LogPath, File.LogPathLen);

    return RET_OK;
}

// log lines counter
uint32_t u32CounterLogLinesTotal(void)
{
    File.LogLinesTotal = 0;
    if(0 >= File.LogPathLen)
    {
        printf("Please input log path first.\n");
        return RET_OK;
    }

    int tmp;
    File.LogFP = fopen(File.LogPath, "rb");
    if(NULL == File.LogFP)
    {
        printf("Open path failed, path: %s\n", File.LogPath);
        return RET_NG;
    }

    while ((tmp = fgetc(File.LogFP)) != EOF)
    {
        if('\n' == tmp)
        {
            File.LogLinesTotal++;
        }
    }

    if(File.LogLinesTotal > 0)
    {
        // The last line has no '\n'
        File.LogLinesTotal++;
    }

    fclose(File.LogFP);
    File.LogFP = NULL;
    return File.LogLinesTotal;
}

uint32_t u32CounterLogLinesSelect(LogLevel level)
{
    LogSearchResult LogSearch = {0};
    uint32_t SelectLines = 0;
    switch (level)
    {
        case LEVEL_VERBOSE:
        case LEVEL_INFO:
        {
            LogSearch.keyword = "INFO";
            break;
        }
        case LEVEL_WARN:
        {
            LogSearch.keyword = "WARN";
            break;
        }
        case LEVEL_ERROR:
        {
            LogSearch.keyword = "ERROR";
            break;
        }
        default:
        {
            return SelectLines;
            break;
        }
    }

    vdKeyWordFileSearch(&LogSearch);

    SelectLines = LogSearch.kwHitCount;
    return SelectLines;
}

// log search
uint8_t u8KeyWordSearch(uint8_t *keyword, size_t keyword_len)
{
    LogSearchResult LogSearch = {0};

    if((NULL == keyword) || (0 >= keyword_len))
    {
        printf("u8KeyWordSearch INVALID INPUT!\n");
        return RET_NG;
    }
    LogSearch.keyword = keyword;
    vdKeyWordFileSearch(&LogSearch);
    return RET_OK;
}

/* Internal functions */
static void vdKeyWordFileSearch(LogSearchResult *ret)
{
    uint8_t cache[LOG_CACHE_LENGTH] = {0};

    if(NULL == ret)
    {
        printf("vdKeyWordFileSearch INVALID INPUT\n");
        return;
    }

    File.LogFP = fopen(File.LogPath, "rb");
    if(NULL == File.LogFP)
    {
        printf("Open path failed, path: %s\n", File.LogPath);
        return;
    }

    while (fgets(cache, LOG_CACHE_LENGTH, File.LogFP) != NULL)
    {
        // printf("read: %s", cache);

        LogSearchResult LogSearchTmp = {0};
        LogSearchTmp = vdInsensitiveSearch(cache, strlen(cache), ret->keyword, strlen(ret->keyword));

        if((strlen(cache) == (LOG_CACHE_LENGTH-1)) && ('\n' != cache[LOG_CACHE_LENGTH-1]))
        {
            // to do
            // if we have to output current line, we should piece together the fragmented caches here.
        }
        else
        {
            // to do
            // in this case, we get the whole line, so no need to piece together.
        }
        ret->kwHitCount = ret->kwHitCount + LogSearchTmp.kwHitCount;

    }

    if (feof(File.LogFP))
    {
        // printf("file end.\n");
    }
    else if (ferror(File.LogFP))
    {
        printf("error happened when reading\n");
    }

    fclose(File.LogFP);
    return;
}

static LogSearchResult vdInsensitiveSearch(const uint8_t *inputLine, size_t inputLen, const uint8_t *keywords, size_t keyLen)
{
    int inputIndex = 0;
    int keyWordIndex = 0;
    LogSearchResult ret = {0};
    if((NULL == inputLine) || (NULL == keywords))
    {
        printf("invalid input");
        return ret;
    }
    
    ret.keyword = keywords;

    while(inputIndex < inputLen)
    {
        if(CaseFlodTable[inputLine[inputIndex]] == CaseFlodTable[keywords[keyWordIndex]])
        {
            keyWordIndex++;
        
        }
        else
        {   
            if(keyWordIndex > 0)
            {
                inputIndex--;
            }
            keyWordIndex = 0;
        }

        if((keyWordIndex >= keyLen) && (inputIndex < inputLen))
        {
            // found target once, have to do something.
            // do something
            printf("read: %s", inputLine);
            ret.kwHitCount++;
            // after doing something, setup keyword index to zero, to continue check next target.
            keyWordIndex = 0;
        }

        // anyway move the cursor to next input character.
        inputIndex++;

    }
    if(ret.kwHitCount > 0)
    {
        ret.line = inputLine;
    }
    return ret;
}
