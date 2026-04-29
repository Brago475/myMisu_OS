#include "timer.h"
#include "idt.h"
#include "ports.h"
#include "process.h"

#define TIME_SLICE_TICKS 10  /* 10 ticks * 10ms = 100ms per slice */

static uint32_t tick = 0;
static uint32_t slice_counter = 0;

static void timer_callback(registers_t* regs) {
    (void) regs;
    tick++;
    slice_counter++;
    process_tick();

    /* Every TIME_SLICE_TICKS, run the scheduler to preempt. */
    if (slice_counter >= TIME_SLICE_TICKS) {
        slice_counter = 0;
        if (process_is_scheduling_enabled()) {
            schedule();
        }
    }
}

void timer_init(uint32_t frequency) {
    register_interrupt_handler(32, timer_callback);

    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return tick;
}
