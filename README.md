# lowfun
x86_64 Linux emulator on every platform. (real basic assembly)
<h1>It executes x86_64 basic instructions on any CPU.</h1>
<h2>code in 'test.s'</h2>
<b>global _start<br>
_start:</b><br>
<em>
  push rbp<br>
  mov rbp,rsp<br>
  sub rsp,0x10<br>
  ;sys_write<br>
  mov rax,1<br>
  mov rdi,1 ;standard output (STDOUT)<br>
  mov rsi,hl<br>
  mov rdx,32<br>
  syscall<br>
  ;sys_exit<br>
  mov rax,60<br>
  mov rdi,1<br>
  syscall<br>
</em>
hl:
  db "Hello from x86 on M1!"  
<hr>
<h1>Running example:</h1>
<img src="https://user-images.githubusercontent.com/59802817/202913593-4075282e-abc7-4360-ae3e-a1586a512b40.png">
<hr>

<h3>docs that helped me while building:</h3>
https://filippo.io/linux-syscall-table<br>
https://www.codeproject.com/Articles/662301/x86-Instruction-Encoding-Revealed-Bit-Twiddling-fo<br>
https://www-user.tu-chemnitz.de/~heha/hsn/chm/x86.chm/x64.htm#Registers<br>
http://ref.x86asm.net/coder64.html<br>
