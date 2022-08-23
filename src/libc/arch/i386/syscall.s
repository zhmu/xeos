.global __SYSCALL

__SYSCALL:
/*
	movl	4(%esp),%eax
	movl	8(%esp),%ebx
	movl	12(%esp),%ecx
	movl	16(%esp),%edx
	movl	20(%esp),%esi
	movl	24(%esp),%edi
*/
	push	%ebx
	push	%ecx
	push	%edx
	push	%esi
	push	%edi

	movl	24(%esp),%eax
	movl	28(%esp),%ebx
	movl	32(%esp),%ecx
	movl	36(%esp),%edx
	movl	40(%esp),%esi
	movl	44(%esp),%edi

	int	$0x30

	pop	%edi
	pop	%esi
	pop	%edx
	pop	%ecx
	pop	%ebx

	movl	%eax,4(%esp)

	ret
