.global divide_error_wrapper, reserved_wrapper, nmi_wrapper
.global breakpoint_wrapper, overflow_wrapper,bound_range_exceeded_wrapper, invalid_opcode_wrapper
.global device_not_available_wrapper, double_fault_wrapper, coprocessor_segment_overrun_wrapper
.global invalid_tss_wrapper, segment_not_present_wrapper, stack_segment_falut_wrapper
.global general_protection_wrapper, page_fault_wrapper, floating_point_error_wrapper
.global alignment_check_wrapper, machine_check_wrapper, simd_float_point_exception_wrapper


divide_error_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call divide_error
	popfl    # pop all flags 
	popal    # pop all registers	
	iret 


reserved_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call reserved
	popfl    # pop all flags 
	popal    # pop all registers
	
	iret

nmi_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call nmi
	popfl    # pop all flags 
	popal    # pop all registers
	iret


breakpoint_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call breakpoint 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret


overflow_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call overflow
	popfl    # pop all flags 
	popal    # pop all registers	
	iret

bound_range_exceeded_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call bound_range_exceeded
	popfl    # pop all flags 
	popal    # pop all registers
	
	iret


invalid_opcode_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call invalid_opcode 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



device_not_available_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call device_not_available 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



double_fault_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call double_fault
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



coprocessor_segment_overrun_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call coprocessor_segment_overrun
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



invalid_tss_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call invalid_tss 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret


segment_not_present_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call segment_not_present
	popfl    # pop all flags 
	popal    # pop all registers	
	iret


stack_segment_falut_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call stack_segment_falut 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret


general_protection_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call general_protection 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



page_fault_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call page_fault
	popfl    # pop all flags 
	popal    # pop all registers	
	iret


floating_point_error_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call floating_point_error 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



alignment_check_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call alignment_check 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



machine_check_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call machine_check 
	popfl    # pop all flags 
	popal    # pop all registers	
	iret



simd_float_point_exception_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call simd_float_point_exception
	popfl    # pop all flags 
	popal    # pop all registers	
	iret

