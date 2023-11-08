#include "esp32-hal.h"
#include "uart.hpp"
#include <Arduino.h>

#define UART0   0
#define UART1   1

UartDevice UartDev;

static void uart0_rx_intr_handler(void *para);

void uart_config(uint8_t uart_no)
{
    if (uart_no == UART1) {
        pinMode(UART1_TX_PIN, OUTPUT);
        digitalWrite(UART1_TX_PIN, HIGH); // Set the TX pin to HIGH (idle state)
    } else {
        /* rcv_buff size if 0x100 */
        UartDev.rcv_buff.BuffState = EMPTY;
        UartDev.rcv_buff.TrigLvl = 1; // Set trigger level (you may need to adjust this)
        UartDev.rcv_buff.pRcvMsgBuff = (uint8_t *)malloc(RX_BUFF_SIZE);
        UartDev.rcv_buff.pWritePos = UartDev.rcv_buff.pRcvMsgBuff;
        UartDev.rcv_buff.pReadPos = UartDev.rcv_buff.pRcvMsgBuff;
        ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, &(UartDev.rcv_buff));
        pinMatrixOutAttach(UART0_TX_PIN, UTX0, false, false);
    }

    Serial.begin(UartDev.baut_rate, SERIAL_8N1, UART0_RX_PIN, UART0_TX_PIN);

    uartDivModify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));

    // clear rx and tx fifo, not ready
    SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

    // set rx fifo trigger
    WRITE_PERI_REG(UART_CONF1(uart_no), (UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S);

    // clear all interrupt
    WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
    // enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA);
}

STATUS uart1_tx_one_char(uint8_t TxChar)
{
    while (true)
    {
        uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(UART1)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
            break;
        }
    }

    WRITE_PERI_REG(UART_FIFO(UART1), TxChar);
    return OK;
}

void uart1_write_char(char c)
{
    if (c == '\n') {
        uart1_tx_one_char('\r');
        uart1_tx_one_char('\n');
    } else if (c == '\r') {
    } else {
        uart1_tx_one_char(c);
    }
}

void uart0_rx_intr_handler(void *para)
{
    RcvMsgBuff *pRxBuff = (RcvMsgBuff *)para;
    uint8_t RcvChar;

    if (UART_RXFIFO_FULL_INT_ST != (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
        return;
    }

    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);

    while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
        RcvChar = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

        *(pRxBuff->pWritePos) = RcvChar;

        if (RcvChar == '\r') {
            pRxBuff->BuffState = WRITE_OVER;
        }

        pRxBuff->pWritePos++;

        if (pRxBuff->pWritePos == (pRxBuff->pRcvMsgBuff + RX_BUFF_SIZE)) {
            pRxBuff->pWritePos = pRxBuff->pRcvMsgBuff;
        }
    }
}

void uart0_tx_buffer(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        Serial.write(buf[i]);
    }
}

void uart_init(UartBautRate uart0_br, UartBautRate uart1_br)
{
    UartDev.baut_rate = uart0_br;
    uart_config(UART0);
    UartDev.baut_rate = uart1_br;
    uart_config(UART1);
    ETS_UART_INTR_ENABLE();
    Serial1.begin(uart1_br, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
    os_install_putc1((void *)uart1_write_char);
}
