global main
extern puts
extern printf
extern memcpy

section .text

name1:
name2:
    mov rsi, rdi
    mov rdi, format
    call printf
    ret

main:
    mov rdi, message
    call name1

    mov rdi, message
    call name2

    ret

section .data

format: db "%s", 10, 0
message: db "I really hope this works", 0
space: db 0
       resb 25
