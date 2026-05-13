.text
  .globl go
go:
  pushq %rbx
  pushq %rbp
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  call _main
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %rbp
  popq %rbx
  retq
_main:
  salq $2, %rax
  sarq $1, %rax
  salq %cl, %rbx
  sarq %cl, %rdx
  retq
