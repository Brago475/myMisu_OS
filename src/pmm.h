#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "multiboot.h"

void pmm_init(multiboot_info_t* mbi);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_used_pages(void);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_memory_kb(void);

#endif
