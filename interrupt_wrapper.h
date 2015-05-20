
#ifndef _INTERRUPT_WRAPPER_H
#define _INTERRUPT_WRAPPER_H

extern void rtc_wrapper(void);
extern void keybrd_wrapper(void);
extern void pit_wrapper(void);
extern void hd_read();
extern void hd_write();
extern void mouse_linkage();

#endif
