LINKER = /lib/ld-linux-x86-64.so.2
OLIB = /usr/lib
DEBUG = -g

syscall_sample.o: syscall_sample.asm
	nasm $(DEBUG) -f elf64 syscall_sample.asm
syscall_sample: syscall_sample.o
	ld -o syscall_sample -dynamic-linker $(LINKER) $(OLIB)/crt1.o $(OLIB)/crti.o -lc syscall_sample.o $(OLIB)/crtn.o

fib.o: fib.asm
	nasm $(DEBUG) -f elf64 fib.asm
fib: fib.o
	ld -o fib -dynamic-linker $(LINKER) $(OLIB)/crt1.o $(OLIB)/crti.o -lc fib.o $(OLIB)/crtn.o

memcpy.o: memcpy.asm
	nasm $(DEBUG) -f elf64 memcpy.asm
memcpy: memcpy.o
	ld -o memcpy -dynamic-linker $(LINKER) $(OLIB)/crt1.o $(OLIB)/crti.o -lc memcpy.o $(OLIB)/crtn.o

alias.o: alias.asm
	nasm $(DEBUG) -f elf64 alias.asm
alias: alias.o
	ld -o alias -dynamic-linker $(LINKER) $(OLIB)/crt1.o $(OLIB)/crti.o -lc alias.o $(OLIB)/crtn.o
