#ifndef _SCHE_H
#define _SCHE_H

#include "lib.h"
#include "rtc.h"
#include "keybrd.h"
#include "file_system_driver.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"

void sche_init();
void scheduling();

int terminal_pid[3];
int cur_index;
int next_index;
int init_flag;

#endif
