#ifndef _LOG_ANALYZER_H_
#define _LOG_ANALYZER_H_

#include "stdint.h"
#include "stdio.h"

#define LOG_MAX_INPUT_LENGTH 0xffffu

typedef enum _LogLevel_{
    LEVEL_VERBOSE             = 0x00u,
    LEVEL_INFO                = 0x01u,
    LEVEL_WARN                = 0x02u,
    LEVEL_ERROR               = 0x03u,
    LEVEL_INVALID             = 0xffu
}LogLevel;


typedef struct _LogSearchResult_{
    // LogLevel level;
    // uint32_t lineNo;
    const uint8_t *keyword;
    const uint8_t *line;
    uint16_t kwHitCount;
}LogSearchResult;

// init
extern uint8_t u8InitLogAnalyzer(void);
// deinit
extern uint8_t u8DeinitLogAnalyzer(void);

// log path input
extern uint8_t u8InputLogPath(uint8_t *LogPath, size_t path_len);

// log lines counter
extern uint32_t u32CounterLogLinesTotal(void);
extern uint32_t u32CounterLogLinesSelect(LogLevel level);

// log search
extern uint8_t u8KeyWordSearch(uint8_t *keyword, size_t keyword_len);

#endif // _LOG_ANALYZER_H_
