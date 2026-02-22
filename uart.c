#include "main.h"
#include "uart.h"
#include "isr.h"

/**
 * PL011_T UART
 *     http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf
 *
 * UARTDR: Data Register   (0x00)
 *    To read received bytes
 *    To write bytes to send
 *    Bit Fields:
 *      15:12 reserved
 *      11:08 error flags
 *       7:0  data bits
 * UARTFR:  Flag Register  (0x18)
 *    Bit Fields:
 *      7:  TXFE  transmit FIFO empty
 *      6:  RXFF  receive FIFO full
 *      5:  TXFF  transmit FIFO full
 *      4:  RXFE  receive FIFO empty
 *      3:  BUSY  set when the UART is busy transmitting data
 */

#define UART_DR 0x00
#define UART_FR 0x18

#define UART_TXFE (1<<7)
#define UART_RXFF (1<<6)
#define UART_TXFF (1<<5)
#define UART_RXFE (1<<4)
#define UART_BUSY (1<<3)
// ======================================
// UART Interrupt Mask Set/Clear Register
#define UART_IMSC 0x038
// UART Interrupt Clear Register
#define UART_ICR  0x044
// Receive Interrupt Mask bit (Bit 4)
#define UART_RXIM (1 << 4)
static void (*user_handler)(void*) = NULL;
static void* user_cookie = NULL;
// ======================================

/*
 * See "uart.h"
 */
int uart_receive(void* uart, uint8_t *b) {
  uint16_t* uart_fr = (uint16_t*) (uart + UART_FR);
  uint16_t* uart_dr = (uint16_t*) (uart + UART_DR);
  if (*uart_fr & UART_RXFE)
    return 0;
  *b = (uint8_t)(*uart_dr & 0xff);
  return 1;
}

/*
 * See "uart.h"
 */
void uart_send(void* uart, uint8_t b) {
  uint16_t* uart_fr = (uint16_t*) (uart + UART_FR);
  uint16_t* uart_dr = (uint16_t*) (uart + UART_DR);
  while (*uart_fr & UART_TXFF)
    ;
  *uart_dr = (uint16_t)b;
}

/*
 * See "uart.h"
 */
void uart_send_string(void* uart, const unsigned char *s) {
  while (*s != '\0') {
    // the following line only works because characters in C
    // are ASCII characters, encoded on 8 bits.
    uart_send(uart, (uint8_t)*s);
    s++;
  }
}

// handler to clear the hardware interrupt source
static void internal_uart_handler(uint32_t irq, void* unused) {
    // 1. Call the user's logic if exists
    if (user_handler) {
        user_handler(user_cookie);
    }

    // 2. Acknowledge/Clear the interrupt at the UART controller
    // without this, the line stays high and the CPU hangs re-entering ISR.
    mmio_write16(UART0, UART_ICR, UART_RXIM); 
}

void uart_enable_interrupt(void (*handler)(void*), void* cookie) {
    user_handler = handler;
    user_cookie = cookie;

    // 1. Register our internal handler with the VIC
    irq_enable(UART0_IRQ, internal_uart_handler, NULL);

    // 2. Unmask the RX interrupt in the UART controller
    // This tells the UART to pull the IRQ line when data arrives.
    uint16_t imsc = mmio_read16(UART0, UART_IMSC);
    imsc |= UART_RXIM;
    mmio_write16(UART0, UART_IMSC, imsc);
}