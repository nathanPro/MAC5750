global main
extern puts
extern printf
extern memcpy

section .text

main:
    mov rdi, message
    call puts 

    mov rdi, space 
    call puts 

    mov rdi, space
    mov rsi, message
    mov rdx, 25
    call memcpy

    mov rdi, space 
    call puts 

    ret

section .data

format: db "%s", 10, 0
message: db "I really hope this works", 0
space: db 0
       resb 25
