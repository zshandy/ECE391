#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

//paging initialization
void paging_init();
void syscall_page(uint32_t addr);
void vid_page(uint32_t addr);
void vid_new(uint32_t addr, int display_index);

#endif /* _RTC_H */
