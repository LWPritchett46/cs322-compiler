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
_curry:
  subq $16, %rsp
  movq %r13, 8(%rsp)
  movq %r12, 0(%rsp)
  movq %rdi, %r12
  movq $5, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %r13
  movq $_curry_arg1, %rdx
  movq %r13, %rdi
  movq $0, %rsi
  movq %rdx, %rdx
  movq $_call_label_ciao_17, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label_ciao_17:
  movq %r13, %rdi
  movq $1, %rsi
  movq %r12, %rdx
  movq $_call_label_ciao_18, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label_ciao_18:
  movq %r13, %rax
  movq 0(%rsp), %r12
  movq 8(%rsp), %r13
  addq $16, %rsp
  retq
_curry_arg1:
  subq $32, %rsp
  movq %r15, 24(%rsp)
  movq %r14, 16(%rsp)
  movq %r13, 8(%rsp)
  movq %r12, 0(%rsp)
  movq %rdi, %r12
  movq %rsi, %r15
  movq $7, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %r14
  movq $_curry_arg2, %r13
  movq %r12, %rdi
  movq $1, %rsi
  movq $_call_label_ciao__ciao_19, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label_ciao__ciao_19:
  movq %rax, %r12
  movq %r14, %rdi
  movq $0, %rsi
  movq %r13, %rdx
  movq $_call_label_ciao__ciao_20, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label_ciao__ciao_20:
  movq %r14, %rdi
  movq $1, %rsi
  movq %r12, %rdx
  movq $_call_label_ciao__ciao_21, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label_ciao__ciao_21:
  movq %r14, %rdi
  movq $2, %rsi
  movq %r15, %rdx
  movq $_call_label_ciao__ciao_22, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label_ciao__ciao_22:
  movq %r14, %rax
  movq 0(%rsp), %r12
  movq 8(%rsp), %r13
  movq 16(%rsp), %r14
  movq 24(%rsp), %r15
  addq $32, %rsp
  retq
_curry_arg2:
  subq $24, %rsp
  movq %r14, 16(%rsp)
  movq %r13, 8(%rsp)
  movq %r12, 0(%rsp)
  movq %rdi, %r12
  movq %rsi, %r14
  movq %r12, %rdi
  movq $1, %rsi
  movq $_call_label_ciao__ciao__ciao_23, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label_ciao__ciao__ciao_23:
  movq %rax, %r13
  movq %r12, %rdi
  movq $2, %rsi
  movq $_call_label_ciao__ciao__ciao_24, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label_ciao__ciao__ciao_24:
  movq %rax, %r12
  movq %r13, %rdi
  movq $0, %rsi
  movq $_call_label_ciao__ciao__ciao_25, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label_ciao__ciao__ciao_25:
  movq %rax, %rcx
  movq %r13, %rdi
  movq %r12, %rsi
  movq %r14, %rdx
  movq $_call_label_ciao__ciao__ciao_26, -8(%rsp)
  subq $8, %rsp
  jmp *%rcx
_call_label_ciao__ciao__ciao_26:
  movq %rax, %rdi
  movq %rdi, %rax
  movq 0(%rsp), %r12
  movq 8(%rsp), %r13
  movq 16(%rsp), %r14
  addq $24, %rsp
  retq
_load:
  movq %rdi, %rdx
  movq %rsi, %rdi
  movq %rdi, %rdi
  imulq $8, %rdi
  movq %rdi, %rdi
  addq $8, %rdi
  movq %rdx, %rdx
  addq %rdi, %rdx
  movq 0(%rdx), %rdi
  movq %rdi, %rax
  retq
_main:
  subq $56, %rsp
  movq %rbx, 40(%rsp)
  movq %rbp, 32(%rsp)
  movq %r15, 24(%rsp)
  movq %r14, 16(%rsp)
  movq %r13, 8(%rsp)
  movq %r12, 0(%rsp)
  movq $7, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %r12
  movq $_plus, %rdx
  movq $_minus, %r13
  movq $_times, %r14
  movq %r12, %rdi
  movq $0, %rsi
  movq %rdx, %rdx
  movq $_call_label3, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label3:
  movq %r12, %rdi
  movq $1, %rsi
  movq %r13, %rdx
  movq $_call_label4, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label4:
  movq %r12, %rdi
  movq $2, %rsi
  movq %r14, %rdx
  movq $_call_label5, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label5:
  movq $7, %r15
  movq $2, %r14
  movq $0, %r13
_call_label0:
  movq %r12, %rdi
  movq %r13, %rsi
  movq $_call_label6, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label6:
  movq %rax, %rbx
  movq %r12, %rdi
  movq %r15, %rsi
  movq %r14, %rdx
  movq $_call_label7, -8(%rsp)
  subq $8, %rsp
  jmp *%rbx
_call_label7:
  movq %rax, %rdi
  movq %rdi, %rdi
  imulq $2, %rdi
  movq %rdi, %rdi
  addq $1, %rdi
  movq %rdi, %rdi
  call print
  movq %r13, %r13
  addq $1, %r13
  cmpq $3, %r13
  setl %dil
  movzbq %dil, %rdi
  cmpq $1, %rdi
  je _call_label0
  movq $7, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %rbp
  movq $0, %r13
_call_label1:
  movq %r12, %rdi
  movq %r13, %rsi
  movq $_call_label8, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label8:
  movq %rax, %rbx
  movq $3, %rdi
  movq $1, %rsi
  call allocate
  movq %rax, %rdi
  movq %rdi, 48(%rsp)
  movq 48(%rsp), %rdi
  movq %rdi, %rdi
  movq $0, %rsi
  movq %rbx, %rdx
  movq $_call_label9, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label9:
  movq 48(%rsp), %rdi
  movq %rdi, %rdi
  movq $_call_label10, -8(%rsp)
  subq $8, %rsp
  jmp _curry
_call_label10:
  movq %rax, %rbx
  movq %rbp, %rdi
  movq %r13, %rsi
  movq %rbx, %rdx
  movq $_call_label11, -8(%rsp)
  subq $8, %rsp
  jmp _store
_call_label11:
  movq %r13, %r13
  addq $1, %r13
  cmpq $3, %r13
  setl %dil
  movzbq %dil, %rdi
  cmpq $1, %rdi
  je _call_label1
  movq $0, %r13
_call_label2:
  movq %rbp, %rdi
  movq %r13, %rsi
  movq $_call_label12, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label12:
  movq %rax, %rbx
  movq %rbx, %rdi
  movq $0, %rsi
  movq $_call_label13, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label13:
  movq %rax, %rdx
  movq %rbx, %rdi
  movq %r15, %rsi
  movq $_call_label14, -8(%rsp)
  subq $8, %rsp
  jmp *%rdx
_call_label14:
  movq %rax, %r12
  movq %r12, %rdi
  movq $0, %rsi
  movq $_call_label15, -8(%rsp)
  subq $8, %rsp
  jmp _load
_call_label15:
  movq %rax, %rdx
  movq %r12, %rdi
  movq %r14, %rsi
  movq $_call_label16, -8(%rsp)
  subq $8, %rsp
  jmp *%rdx
_call_label16:
  movq %rax, %rdi
  movq %rdi, %rdi
  imulq $2, %rdi
  movq %rdi, %rdi
  addq $1, %rdi
  movq %rdi, %rdi
  call print
  movq %r13, %r13
  addq $1, %r13
  cmpq $3, %r13
  setl %dil
  movzbq %dil, %rdi
  cmpq $1, %rdi
  je _call_label2
  movq 0(%rsp), %r12
  movq 8(%rsp), %r13
  movq 16(%rsp), %r14
  movq 24(%rsp), %r15
  movq 32(%rsp), %rbp
  movq 40(%rsp), %rbx
  addq $56, %rsp
  retq
_minus:
  movq %rdi, %rdi
  movq %rsi, %rdi
  movq %rdx, %rsi
  movq %rdi, %rdi
  subq %rsi, %rdi
  movq %rdi, %rax
  retq
_plus:
  movq %rdi, %rdi
  movq %rsi, %rdi
  movq %rdx, %rsi
  movq %rdi, %rdi
  addq %rsi, %rdi
  movq %rdi, %rax
  retq
_store:
  movq %rdi, %rdi
  movq %rsi, %rsi
  movq %rdx, %rdx
  movq %rsi, %rsi
  addq $1, %rsi
  lea (%rdi, %rsi, 8), %rdi
  movq %rdx, 0(%rdi)
  retq
_times:
  movq %rdi, %rdi
  movq %rsi, %rdi
  movq %rdx, %rsi
  movq %rdi, %rdi
  imulq %rsi, %rdi
  movq %rdi, %rax
  retq
