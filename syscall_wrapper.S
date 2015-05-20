.text # The text section is used for keeping the actual code
.global syscall_wrapper

#system call linkage
syscall_wrapper:
	# caller save reg
	pushf
	pushl %esi
	pushl %edi
	pushl %ebp
	
	# calculate the function index
	decl %eax
	cmpl $0, %eax
	jl Invalid
	cmpl $9, %eax
	jg Invalid

	#argument
	pushl %edx
	pushl %ecx
	pushl %ebx
	cli
	call *jump_table(, %eax, 4);
	
	#pop the argument
	popl %ebx
	popl %ecx
	popl %edx

DONE:
	#restore the caller save reg
	popl %ebp
	popl %edi
	popl %esi
	popf
	iret

# Invalid index for system call
Invalid:
	movl $-1, %eax
	jmp DONE

jump_table:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
