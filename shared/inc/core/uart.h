#ifndef LOCAL_UART_H_
#define LOCAL_UART_H_

#include "common-includes.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define UART_PORT           (GPIOA)
#define UART_TX_PIN         (GPIO2)
#define UART_RX_PIN         (GPIO3)

int _write(int file, char *ptr, int len);

void coreUartSetup(uint32_t baudrate);
void coreUartWrite(uint8_t *data, uint32_t len);
void coreUartWriteByte(uint8_t byte);
uint32_t coreUartRead(uint8_t *data, uint32_t len);
uint8_t coreUartReadByte(void);
bool coreUartDataAvailable(void);

#endif