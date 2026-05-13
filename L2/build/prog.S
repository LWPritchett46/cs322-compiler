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
  subq $8, %rsp
  movq %r12, 0(%rsp)
  movq $_call_label0, -8(%rsp)
  subq $8, %rsp
  jmp _createArray1
_call_label0:
  movq %rax, %r12
  movq $_call_label1, -8(%rsp)
  subq $8, %rsp
  jmp _createArray2
_call_label1:
  movq %rax, %rsi
  movq %r12, %rdi
  movq %rsi, %rsi
  movq $_call_label2, -8(%rsp)
  subq $8, %rsp
  jmp _zip
_call_label2:
  movq %rax, %rdi
  movq %rdi, %rdi
  call print
  movq 0(%rsp), %r12
  addq $8, %rsp
  retq
_createArray1:
  movq $13, %rdi
  movq %rdi, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %rdx
  movq $0, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $3, 0(%rdi)
  movq $1, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $7, 0(%rdi)
  movq $2, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $5, 0(%rdi)
  movq $3, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $5, 0(%rdi)
  movq $4, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $5, 0(%rdi)
  movq $5, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $5, 0(%rdi)
  movq %rdx, %rax
  retq
_createArray2:
  movq $13, %rdi
  movq %rdi, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %rdx
  movq $0, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $9, 0(%rdi)
  movq $1, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $43, 0(%rdi)
  movq $2, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $1, 0(%rdi)
  movq $3, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $13, 0(%rdi)
  movq $4, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $15, 0(%rdi)
  movq $5, %rdi
  imulq $8, %rdi
  movq %rdi, %rsi
  addq $8, %rsi
  movq %rdx, %rdi
  addq %rsi, %rdi
  movq $5, 0(%rdi)
  movq %rdx, %rax
  retq
_zip:
  subq $48, %rsp
  movq %r12, 40(%rsp)
  movq %r13, 32(%rsp)
  movq %r14, 24(%rsp)
  movq %r15, 16(%rsp)
  movq %rbp, 8(%rsp)
  movq %rbx, 0(%rsp)
  movq %rdi, %r14
  movq %rsi, %r13
  movq 0(%r14), %r12
  movq 0(%r13), %rdi
  cmpq %rdi, %r12
  sete %dil
  movzbq %dil, %rdi
  cmpq $1, %rdi
  je _call_label_ciao_4
  movq 0(%rsp), %rbx
  movq 8(%rsp), %rbp
  movq 16(%rsp), %r15
  movq 24(%rsp), %r14
  movq 32(%rsp), %r13
  movq 40(%rsp), %r12
  addq $48, %rsp
  retq
_call_label_ciao_4:
  movq %r12, %rdi
  salq $1, %rdi
  movq %rdi, %rdi
  addq $1, %rdi
  movq %rdi, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %r15
  movq $0, %rbp
_call_label_ciao_5:
  movq %rbp, %rbx
  addq $1, %rbx
  movq %rbx, %rbx
  imulq $8, %rbx
  movq $5, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %rdx
  movq %rdx, %rsi
  movq %rsi, %rsi
  addq $8, %rsi
  movq %r14, %rdi
  addq %rbx, %rdi
  movq 0(%rdi), %rdi
  movq %rdi, 0(%rsi)
  movq %r13, %rdi
  addq %rbx, %rdi
  movq 0(%rdi), %rdi
  movq %rsi, %rsi
  addq $8, %rsi
  movq %rdi, 0(%rsi)
  movq %r15, %rdi
  addq %rbx, %rdi
  movq %rdx, 0(%rdi)
  movq %rbp, %rbp
  addq $1, %rbp
  cmpq %r12, %rbp
  setl %dil
  movzbq %dil, %rdi
  cmpq $1, %rdi
  je _call_label_ciao_5
  jmp _call_label_ciao_6
_call_label_ciao_6:
  movq %r15, %rax
  movq 0(%rsp), %rbx
  movq 8(%rsp), %rbp
  movq 16(%rsp), %r15
  movq 24(%rsp), %r14
  movq 32(%rsp), %r13
  movq 40(%rsp), %r12
  addq $48, %rsp
  retq
