#ifndef LOCAL_UART_H_
#define LOCAL_UART_H_

#include "common-includes.h"

void coreUartSetup(uint32_t baudrate);
void coreUartWrite(uint8_t *data, uint32_t len);
void coreUartWriteByte(uint8_t byte);
uint32_t coreUartRead(uint8_t *data, uint32_t len);
uint8_t coreUartReadByte(void);
bool coreUartDataAvailable(void);

#endif