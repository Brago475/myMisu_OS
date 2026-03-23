/* Multiboot header constants */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* Multiboot flag field */
.set MAGIC,    0x1BADB002       /* magic number for bootloader to find */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum: magic + flags + checksum = 0 */

/* Declare the Multiboot header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Reserve a 16KB stack */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

/* Kernel entry point */
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the stack pointer */
    mov $stack_top, %esp

    /* Push multiboot info pointer and magic number for kernel_main */
    push %ebx    /* multiboot info struct pointer */
    push %eax    /* multiboot magic number */

    /* Call the C kernel */
    call kernel_main

    /* If kernel_main returns, hang the CPU */
    cli
hang:
    hlt
    jmp hang

.size _start, . - _start
