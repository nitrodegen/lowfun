global _start

_start:
  push rbp
  mov rbp,rsp
  sub rsp,0x10
  ;sys_write
  mov rax,1
  mov rdi,1 ;standard output (STDOUT)
  mov rsi,hl
  mov rdx,32
  syscall
  ;sys_exit
  mov rax,60
  mov rdi,1
  syscall

hl:
  db "Hello from x86 on M1!"  
