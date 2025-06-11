#include "stm32f1xx.h"
#include "itm_console.h"

/**
 * @brief Optional ITM init (usually handled by debugger)
 */
void ITM_Console_Init(void)
{
    // Optional manual init — usually debugger does this
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR = 0xC5ACCE55;
    ITM->TCR = ITM_TCR_ITMENA_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk;
    ITM->TER = 0x01;  // Enable stimulus port 0
}

/**
 * @brief Syscall hook used by printf, puts, etc. (newlib-nano)
 */
int _write(int file, char *ptr, int len)
{
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) &&       // ITM enabled
        (ITM->TER & (1UL << 0)))                 // Port 0 enabled
    {
        for (int i = 0; i < len; i++)
        {
            ITM_SendChar((uint32_t)*ptr++);
        }
    }
    return len;
}
