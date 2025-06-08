/* STM32F103 implementation for W5500 chip select using PB12 */
#include "stm32f1xx_hal.h"

/* Define SPI handle for STM32 HAL */
extern SPI_HandleTypeDef hspi2;

/////////////
//M20150401 : Remove ; in the default callback function such as wizchip_cris_enter(), wizchip_cs_select() and etc.
/////////////

/**
 * @brief Default function to enable interrupt.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	  wizchip_cris_enter(void)           {};
void 	  wizchip_cris_enter(void)           {



   
}

/**
 * @brief Default function to disable interrupt.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	  wizchip_cris_exit(void)          {};
void 	  wizchip_cris_exit(void)          {




}

/**
 * @brief Default function to select chip.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	wizchip_cs_select(void)            {};
void 	wizchip_cs_select(void)
{
    /* Set PB12 (CS pin) to LOW for chip select */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

/**
 * @brief Default function to deselect chip.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	wizchip_cs_deselect(void)          {};
void 	wizchip_cs_deselect(void)
{
    /* Set PB12 (CS pin) to HIGH for chip deselect */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

/**
 * @brief Default function to read in direct or indirect interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
// M20150601 : Rename the function for integrating with W5300
// uint8_t wizchip_bus_readbyte(uint32_t AddrSel) { return * ((volatile uint8_t *)((ptrdiff_t) AddrSel)); }
iodata_t wizchip_bus_readdata(uint32_t AddrSel) 
{ 
    uint8_t rx_data = 0;
    /* W5500 with STM32F103 using SPI2 (PB12-PB15) */

    /* Chip select, transmit control bytes, receive data, chip deselect */
    HAL_SPI_Receive(&hspi2, &rx_data, 1, HAL_MAX_DELAY);

    return rx_data;
}

/**
 * @brief Default function to write in direct or indirect interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
// M20150601 : Rename the function for integrating with W5300
// void 	wizchip_bus_writebyte(uint32_t AddrSel, uint8_t wb)  { *((volatile uint8_t*)((ptrdiff_t)AddrSel)) = wb; }
void 	wizchip_bus_writedata(uint32_t AddrSel, iodata_t wb)  
{ 
    uint8_t tx_data = (uint8_t)wb;
    /* W5500 with STM32F103 using SPI2 (PB12-PB15) */
    
    /* Chip select, transmit data byte, chip deselect */
    HAL_SPI_Transmit(&hspi2, &tx_data, 1, HAL_MAX_DELAY);

}

/**
 * @brief Default function to read in SPI interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//uint8_t wizchip_spi_readbyte(void)        {return 0;};
uint8_t wizchip_spi_readbyte(void) {
   uint8_t rx_data = 0;
   
   /* Read one byte via SPI2 */
   HAL_SPI_Receive(&hspi2, &rx_data, 1, HAL_MAX_DELAY);
   
   return rx_data;
}

/**
 * @brief Default function to write in SPI interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	wizchip_spi_writebyte(uint8_t wb) {};
void 	wizchip_spi_writebyte(uint8_t wb) {
   uint8_t tx_data = wb;
   
   /* Write one byte via SPI2 */
   HAL_SPI_Transmit(&hspi2, &tx_data, 1, HAL_MAX_DELAY);
}

/**
 * @brief Default function to burst read in SPI interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) 	{}; 
void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) 	{
   /* Read multiple bytes via SPI2 */
   HAL_SPI_Receive(&hspi2, pBuf, len, HAL_MAX_DELAY);
}

/**
 * @brief Default function to burst write in SPI interface.
 * @note This function help not to access wrong address. If you do not describe this function or register any functions,
 * null function is called.
 */
//void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {};
void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {
   /* Write multiple bytes via SPI2 */
   HAL_SPI_Transmit(&hspi2, pBuf, len, HAL_MAX_DELAY);
}
