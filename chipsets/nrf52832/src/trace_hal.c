#include "core_debug_reg.h"
#include "core_itm_reg.h"
#include "core_tpiu_reg.h"
#include "clock_prv.h"

#include "utils.h"

#include "chip_cfg.h"

#include <stdarg.h>
#include <stdbool.h>

static void parseFormatSpecifier(char const ** currentChar, va_list args);
static void putCharacter(char const character);

void trace_init(void)
{
    CORE_DEBUG.DEMCR.TRCENA = ENABLED;

    CORE_TPIU.TPIU_SPPR.TXMODE = TPIU_SPPR_TXMODE_SWO_NRZ;
    CORE_TPIU.TPIU_ACPR = (CORE_CLOCK_FREQUENCY_HZ/CORE_CLOCK_TRACEPORT_SPEED_HZ) - 1;

    CORE_ITM.ITM_TCR.ITMENA = ENABLED;
    CORE_ITM.ITM_TER = 1U;
}

void traceOut(char const * const formatString, ...)
{
    va_list args;
    va_start(args, formatString);
    char const * currentChar = formatString;

    while(*currentChar != '\0')
    {
        parseFormatSpecifier(&currentChar, args);

        putCharacter(*currentChar);
        ++currentChar;
    }
    va_end(args);
}

static void parseFormatSpecifier(char const ** currentChar, va_list args)
{
    static bool reentrantGuard = false;
    if(reentrantGuard || (**currentChar != '%'))
    {
        return;
    }
    reentrantGuard = true;
    ++*currentChar;
    switch(**currentChar)
    {
        case '%':
        {
            putCharacter(**currentChar);
        }
        break;
        case 's':
        {
            char const * const string = va_arg(args, char const * const);
            traceOut(string);
        }
        break;
    }
    ++*currentChar;
    reentrantGuard = false;
}

static void putCharacter(char const character)
{
    while( CORE_ITM.ITM_STIM[0].STATUS == CORE_ITM_FIFO_FULL ) { }
    CORE_ITM.ITM_STIM[0].WRITE = character;
}