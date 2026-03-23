/* ISR stubs for CPU exceptions 0-31 */

.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli
    push $0          /* Dummy error code */
    push $\num       /* Interrupt number */
    jmp isr_common
.endm

.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli
    push $\num       /* Interrupt number (error code already pushed by CPU) */
    jmp isr_common
.endm

.macro IRQ irq_num, int_num
.global irq\irq_num
irq\irq_num:
    cli
    push $0
    push $\int_num
    jmp irq_common
.endm

/* CPU Exceptions */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

/* Hardware IRQs */
IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* Common ISR handler */
.extern isr_handler
isr_common:
    pusha            /* Push all registers */
    mov %ds, %ax
    push %eax        /* Save data segment */

    mov $0x10, %ax   /* Load kernel data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp        /* Push pointer to registers_t */
    call isr_handler
    add $4, %esp

    pop %eax         /* Restore data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp     /* Remove int_no and err_code */
    sti
    iret

/* Common IRQ handler */
.extern irq_handler
irq_common:
    pusha
    mov %ds, %ax
    push %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler
    add $4, %esp

    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp
    sti
    iret

/* IDT flush */
.global idt_flush
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret
