.DATA
temp_number DQ 0


.CODE


__syscall proc
	mov rax,temp_number;
	mov r10,rcx;
	syscall
	ret;
__syscall endp;

END