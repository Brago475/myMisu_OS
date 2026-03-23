#include "pmm.h"
#include "string.h"
#include "kprintf.h"

#define PAGE_SIZE 4096
#define MAX_PAGES 32768    /* Support up to 128MB */
#define BITMAP_SIZE (MAX_PAGES / 8)

static uint8_t bitmap[BITMAP_SIZE];
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;

static inline void pmm_set_bit(uint32_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static inline void pmm_clear_bit(uint32_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline int pmm_test_bit(uint32_t page) {
    return bitmap[page / 8] & (1 << (page % 8));
}

void pmm_init(multiboot_info_t* mbi) {
    /* Mark all pages as used (safe default) */
    memset(bitmap, 0xFF, BITMAP_SIZE);
    used_pages = MAX_PAGES;

    /* Use mem_upper from multiboot (KB above 1MB) */
    uint32_t total_kb = mbi->mem_lower + mbi->mem_upper + 1024;
    total_pages = total_kb / 4;  /* 4KB per page */
    if (total_pages > MAX_PAGES) total_pages = MAX_PAGES;

    /* Walk the multiboot memory map */
    if (mbi->flags & (1 << 6)) {
        multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*) mbi->mmap_addr;
        uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)entry < mmap_end) {
            if (entry->type == 1) {  /* Available RAM */
                uint32_t base = (uint32_t) entry->addr;
                uint32_t length = (uint32_t) entry->len;

                /* Align to page boundaries */
                uint32_t start = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                uint32_t end = (base + length) & ~(PAGE_SIZE - 1);

                for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
                    uint32_t page = addr / PAGE_SIZE;
                    if (page < MAX_PAGES && page > 0) {
                        pmm_clear_bit(page);
                        used_pages--;
                    }
                }
            }
            entry = (multiboot_mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    }

    /* Reserve first 1MB (BIOS, bootloader, etc.) */
    for (uint32_t i = 0; i < 256; i++) {  /* 256 pages = 1MB */
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            used_pages++;
        }
    }

    /* Reserve kernel pages (1MB to _kernel_end) */
    extern uint32_t _kernel_end;
    uint32_t kernel_end_page = ((uint32_t)&_kernel_end) / PAGE_SIZE + 1;
    for (uint32_t i = 256; i <= kernel_end_page && i < MAX_PAGES; i++) {
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            used_pages++;
        }
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < MAX_PAGES / 8; i++) {
        if (bitmap[i] != 0xFF) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (!(bitmap[i] & (1 << bit))) {
                    uint32_t page = i * 8 + bit;
                    pmm_set_bit(page);
                    used_pages++;
                    return page * PAGE_SIZE;
                }
            }
        }
    }
    return 0;  /* Out of memory */
}

void pmm_free_page(uint32_t addr) {
    uint32_t page = addr / PAGE_SIZE;
    if (page < MAX_PAGES && pmm_test_bit(page)) {
        pmm_clear_bit(page);
        used_pages--;
    }
}

uint32_t pmm_get_total_pages(void) { return total_pages; }
uint32_t pmm_get_used_pages(void)  { return used_pages; }
uint32_t pmm_get_free_pages(void)  { return total_pages > used_pages ? total_pages - used_pages : 0; }
uint32_t pmm_get_total_memory_kb(void) { return total_pages * 4; }
