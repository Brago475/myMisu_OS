# MyMisu OS

A bare-metal x86 operating system built from scratch in C.

## Download

Go to Releases and download mymisu.iso

## Run in QEMU

qemu-system-i386 -cdrom mymisu.iso -m 128M

## Flash to USB

1. Download Rufus from rufus.ie
2. Flash mymisu.iso to a USB drive
3. Boot any PC from the USB

## Login

Username: misu
Password: misu

## Features

- Login screen with authentication
- 35+ shell commands
- Ramdisk filesystem (ls, cat, touch, write, mkdir, rm)
- Process management (ps, spawn, kill)
- Physical memory manager (meminfo, alloc)
- 5 system calls (write, read, mkdir, getpid, uptime)
- Text editor, calculator, notes app
- Games: snake, tic-tac-toe, hangman, rock-paper-scissors, pong
- Visual menu, file browser, book reader, system monitor
- Desktop widgets with live clock
- Boot animation
- Neofetch with cat ASCII art

## Built With

- C + x86 Assembly
- GCC 13.2.0 (i686-elf cross compiler)
- GRUB Multiboot
- Built with AI (Claude) assistance

## Team

- James Mardi
- Danny
