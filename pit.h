#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "rtc.h"
#include "keybrd.h"
#include "file_system_driver.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"

void pit_init();
void send_pit_eoi();

#endif
