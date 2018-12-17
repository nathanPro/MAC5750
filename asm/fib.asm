global main
extern printf

section .text


print:
    enter 0, 0
    mov rsi, rdi
    mov rdi, format
    call printf
    leave
    ret

fib:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    cmp rdi, 2
    jge not_base

    mov rax, 1
    jmp prologue

not_base:
    sub rdi, 1
    mov [rsp + 8], rdi
    call fib
    mov [rsp + 0], rax

    mov rdi, [rsp + 8]
    sub rdi, 1
    call fib
    add rax, [rsp + 0]
    jmp prologue

prologue:
    leave
    ret

main:
        xor rax, rax
    check:
        cmp rax, 20
        je out

        mov rdi, rax
        push rax

        call fib
        mov rdi, rax
        call print

        pop rax
        add rax, 1
        jmp check
    out:
    xor rax, rax
    ret

section .data

format: db "%d", 10, 0
