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

main:
    xor rax, rax
check:
    cmp rax, [number]
    jg out

    mov rdi, rax

    push rax
    call print
    pop rax

    add rax, 10h
    jmp check
out:
    xor rax, rax
    ret

section .data

number: dq 40h
format: db "%d", 10, 0
