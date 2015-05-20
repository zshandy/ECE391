#ifndef _EXCEPTION_H
#define _EXCEPTION_H

void init_exception(void);

void divide_error(void);
void reserved(void);
void nmi(void);
void breakpoint(void);
void overflow(void);
void bound_range_exceeded(void);
void invalid_opcode(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_tss(void);
void segment_not_present(void);
void stack_segment_falut(void);
void general_protection(void);
void page_fault(void);
void floating_point_error(void);
void alignment_check(void);
void machine_check(void);
void simd_float_point_exception(void);

#endif
