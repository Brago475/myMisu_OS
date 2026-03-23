.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax    /* Get pointer to GDT ptr struct */
    lgdt (%eax)          /* Load GDT */

    mov $0x10, %ax       /* Kernel data segment = 0x10 */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $.flush  /* Far jump to kernel code segment = 0x08 */
.flush:
    ret
