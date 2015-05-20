
#ifndef _EXCEPTION_WRAPPERS_H
#define _EXCEPTION_WRAPPERS_H

extern void divide_error_wrapper(void);
extern void reserved_wrapper(void);
extern void nmi_wrapper(void);
extern void breakpoint_wrapper(void);
extern void overflow_wrapper(void);

extern void bound_range_exceeded_wrapper(void);
extern void invalid_opcode_wrapper(void);
extern void device_not_available_wrapper(void);
extern void double_fault_wrapper(void);
extern void invalid_tss_wrapper(void);

extern void coprocessor_segment_overrun_wrapper(void);
extern void segment_not_present_wrapper(void);
extern void stack_segment_falut_wrapper(void);
extern void general_protection_wrapper(void);
extern void page_fault_wrapper(void);

extern void floating_point_error_wrapper(void);
extern void alignment_check_wrapper(void);
extern void machine_check_wrapper(void);
extern void simd_float_point_exception_wrapper(void);



#endif

