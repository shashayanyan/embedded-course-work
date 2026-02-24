#include "timer.h"
#include "isr.h"
#include "main.h"

#define TIMER0_BASE 0x101E2000

//Register Offsets
#define TIMER_LOAD    0x00
#define TIMER_CONTROL 0x08
#define TIMER_INTCLR  0x0C

//Control Register Bits
#define TIMER_EN      (1 << 7) // Enable Timer
#define TIMER_MODE    (1 << 6) // Periodic Mode
#define TIMER_INTEN   (1 << 5) // Enable Interrupts
#define TIMER_32BIT   (1 << 1) // 32-bit counter

volatile uint64_t system_ticks = 0;
volatile uint32_t system_seconds = 0; // NEW: seconds for status bar
static void timer_isr(uint32_t irq, void* cookie) {
    // 1. Advance the global system clock
    system_ticks++;

    // seconds counter
    static uint32_t ms_counter = 0;
    ms_counter++;
    if (ms_counter >= 1000) {
        system_seconds++;
        ms_counter = 0;
    }

    // 2. Clear the interrupt at the timer hardware level
    mmio_write32((void*)TIMER0_BASE, TIMER_INTCLR, 1);
}

void timer_init(void) {
    system_ticks = 0;

    // 1. Register the interrupt handler with the VIC
    // TIMER(0&1) IRQ = 4
    irq_enable(4, timer_isr, NULL);

    // 2. Confgure the SP804 Timer 0
    // 1MHz ==>
    // We want 1ms interrupts, so we load 1000 (0x3E8).
    mmio_write32((void*)TIMER0_BASE, TIMER_LOAD, 1000);

    // 3. Start the timer: Enabled, Periodic, Interrupt Enabled, 32-bit
    uint32_t ctrl = TIMER_EN + TIMER_MODE + TIMER_INTEN + TIMER_32BIT;
    // Can also just use "|" instead of "+". Bits are distinct so it wouldn't change anything (?) 
    mmio_write32((void*)TIMER0_BASE, TIMER_CONTROL, ctrl);
}