#include "main.h"
#include "isr.h"
#include "isr-mmio.h"

// assembly functions
extern void _irqs_setup(void);
extern void _irqs_enable(void);
extern void _irqs_disable(void);
extern void _wfi(void);

// Structure to hold registered handlers
struct handler_t {
    void (*callback)(uint32_t, void*);
    void* cookie;
};

// Array of handlers for the 32 possible interrupt sources
static struct handler_t handlers[NIRQS];

// ============================================
// Helpers: Read/Write a 32-bit register from/to the VIC
static uint32_t vic_read(uint32_t offset) {
    return *((volatile uint32_t*)(VIC_BASE_ADDR + offset));
}

static void vic_write(uint32_t offset, uint32_t value) {
    *((volatile uint32_t*)(VIC_BASE_ADDR + offset)) = value;
}
// ============================================

/*
 * VIC behavior:
 */
void irqs_setup(){
    _irqs_setup();
}
void irqs_enable(){
    _irqs_enable();
}
void irqs_disable(){
    _irqs_disable();
}
void wfi(void){
    _wfi();
}

void irq_init(void) {
    // Disable all interrupts at the VIC level initially
    vic_write(VICINTCLEAR, 0xFFFFFFFF);
    for (int i = 0; i < NIRQS; i++) {
        handlers[i].callback = NULL;
    }
}

/*
 * Enable the given interrupt,
 * like UART0_IRQ
 */
void irq_enable(uint32_t irq,void(*callback)(uint32_t,void*),void*cookie){
    if (irq >= NIRQS) return;
    
    // 1. Register the handler
    handlers[irq].callback = callback;
    handlers[irq].cookie = cookie;
    
    // 2. Enable the specific interrupt in the VIC
    // VICINTENABLE (0x010) - Writing 1 enables
    vic_write(VICINTENABLE, (1 << irq));
}

/*
 * Disable the given interrupt,
 * like UART0_IRQ
 */
void irq_disable(uint32_t irq){
    if (irq >= NIRQS) return;

    // 1. Remove the software handler (optional safety)
    handlers[irq].callback = NULL;
    handlers[irq].cookie = NULL;

    // 2. Disable the interrupt in hardware
    // We write 1 to VICINTENCLEAR (Offset 0x014) to clear the enable bit.
    // Writing 0 has no effect.
    vic_write(VICINTCLEAR, (1 << irq));
}




// This is called by assembly _isr_handler
void isr(void) {
    //kprintf("[IRQ] "); // making sure it works
    // 1. Read VICIRQSTATUS to see which interrupts are active
    uint32_t status = vic_read(VICIRQSTATUS);

    // 2. Dispatch to the registered handlers
    for (int i = 0; i < NIRQS; i++) {
        if (status & (1 << i)) {
            if (handlers[i].callback) {
                handlers[i].callback(i, handlers[i].cookie);
            }
        }
    }
    // Note: VIC doesn't require an explicit ACK at the controller level,
    // but the device (e.g., UART) MUST be cleared in the handler.
}