CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

ASM_SRCS = src/boot.s src/gdt_flush.s src/isr.s
C_SRCS = src/kernel.c src/vga.c src/gdt.c src/idt.c src/timer.c \
         src/keyboard.c src/string.c src/kprintf.c src/shell.c \
         src/pmm.c src/syscall.c src/fs.c src/process.c

ASM_OBJS = $(ASM_SRCS:.s=.o)
C_OBJS = $(C_SRCS:.c=.o)
OBJS = $(ASM_OBJS) $(C_OBJS)

KERNEL = build/mymisu.bin
ISO = build/mymisu.iso

.PHONY: all clean run debug

all: $(ISO)

%.o: %.s
	$(AS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "=== Kernel linked: $(KERNEL) ==="

$(ISO): $(KERNEL)
	@mkdir -p iso/boot
	cp $(KERNEL) iso/boot/mymisu.bin
	grub-mkrescue -o $(ISO) iso/ 2>/dev/null
	@echo "=== ISO created: $(ISO) ==="
	@echo ""
	@echo "Run 'make run' to boot in QEMU!"

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M

debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M -s -S &
	@echo "Run: i686-elf-gdb build/mymisu.bin -ex 'target remote :1234'"

clean:
	rm -f $(OBJS)
	rm -rf build/
	rm -f iso/boot/mymisu.bin
	@echo "Cleaned!"
