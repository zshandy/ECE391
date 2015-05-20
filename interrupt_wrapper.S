.text # The text section is used for keeping the actual code

.global rtc_wrapper, keybrd_wrapper, pit_wrapper, hd_read, hd_write, mouse_linkage

# saves all registers and flags before rtc_handler, restores all registers and flags after
rtc_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call rtc_handler   
	popfl    # pop all flags 
	popal    # pop all registers
	
	iret


# saves all registers and flags before keyboard_handler, restores all registers and flags after
keybrd_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call keyboard_handler
	popfl    # pop all flags 
	popal    # pop all registers
	
	iret

pit_wrapper:
	pushal
	pushfl
	call pit_handler
	popfl
	popal

	iret


hd_read:
.intel_syntax noprefix
	push ds
	push es
	push fs
	push gs
	push ss
	push eax
	push esi
	push edi
	push ebp
	push edx 		
	push ecx
	push ebx

.att_syntax prefix
	movw    $0x0018, %dx
	movw    %dx, %ds
	movw    %dx, %es
	movw    %dx, %fs
	movw    %dx, %gs
	movw    %dx, %ss
.intel_syntax noprefix

	mov dx,0x1F6
	mov al,0xA0
	out dx,al
	
	mov dx,0x1F2            #Sector count port
	mov al,1               #Read CH sectors
	out dx,al

	mov dx,0x1F3            #Sector number port
	mov al,bl               #BL is sector index
	out dx,al

	mov dx,0x1F4            #Cylinder low port
	mov al,cl
	out dx,al

	mov dx,0x1F5            #Cylinder high port
	mov al,ch
	out dx,al

	mov dx,0x1F7            #Command port
	mov al,0x20              #Read with retry.
	out dx,al
	
.still_going_read:
	in al,dx
	test al,8               #the sector buffer requires servicing.
	jz .still_going_read         #until the sector buffer is ready.

	cld
	mov dx,0x1F0
	mov cx,256
	rep insw                #in to [EDI]
	
	mov dx,0x1F7            #status port
	in al,dx
	
	
	pop ebx
	pop ecx
	pop edx
	pop ebp
	pop edi
	pop esi
	pop eax
	pop ss
	pop gs
	pop fs
	pop es
	pop ds
	ret
	
##########################################################

hd_write:
.intel_syntax noprefix
	push ds
	push es
	push fs
	push gs
	push ss
	push eax
	push esi
	push edi
	push ebp
	push edx 		
	push ecx
	push ebx

.att_syntax prefix
	movw $0x18,%dx		#set to kernel space ds
	movw %dx,%ds
	movw %dx,%es
	movw %dx,%fs
	movw %dx,%gs
	movw %dx, %ss
.intel_syntax noprefix

	mov dx,0x1F6
	mov al,0xA0
	out dx,al
	
	mov dx,0x1F2            #Sector count port
	mov al,1               #Read CH sectors
	out dx,al

	mov dx,0x1F3            #Sector number port
	mov al,bl               #BL is sector index
	out dx,al

	mov dx,0x1F4            #Cylinder low port
	mov al,cl
	out dx,al

	mov dx,0x1F5            #Cylinder high port
	mov al,ch
	out dx,al

	mov dx,0x1F7            #Command port
	mov al,0x30              #Write with retry.
	out dx,al
	
.still_going_write:
	in al,dx
	test al,8               #the sector buffer requires servicing.
	jz .still_going_write         #until the sector buffer is ready.

	cld
	mov dx,0x1F0
	mov cx,256
	rep outsw                #out to [ESI]
	
	mov dx,0x1F7            #Command port
	in al,dx
	
	
	pop ebx
	pop ecx
	pop edx
	pop ebp
	pop edi
	pop esi
	pop eax
	pop ss
	pop gs
	pop fs
	pop es
	pop ds
	ret

.att_syntax prefix
mouse_linkage:
	pushal
	pushf
	cli
	call mouse_handler
	popfl
	popal
	iret

