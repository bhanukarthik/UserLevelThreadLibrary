

.global thread_start
.global thread_switch

thread_start:
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	movq %rsp,(%rdi)
	movq (%rsi),%rsp
	pushq $thread_wrap
	movq 8(%rsi),%rdi
	movq 16(%rsi),%rsi
	ret

thread_switch:
        pushq %rbx
        pushq %rbp
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15
	movq %rsp,(%rdi)
        movq (%rsi),%rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	ret
