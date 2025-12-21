    .data
    .align 4

dev_name:
    .asciz "/dev/ttyAMA0"

    .align 4
options:
    .word 0x00000000      @ c_iflag
    .word 0x00000000      @ c_oflag
    .word 0x00000000      @ c_cflag
    .word 0x00000000      @ c_lflag
    .byte 0x00            @ c_line

    .word 0x00000000      @ c_cc[0-3]
    .word 0x00000000      @ c_cc[4-7]
    .word 0x00000000      @ c_cc[8-11]
    .word 0x00000000      @ c_cc[12-15]
    .word 0x00000000      @ c_cc[16-19]
    .word 0x00000000      @ c_cc[20-23]
    .word 0x00000000      @ c_cc[24-27]
    .word 0x00000000      @ c_cc[28-31]

    .byte 0x00            @ padding
    .hword 0x0000         @ padding
    .word 0x00000000      @ c_ispeed
    .word 0x00000000      @ c_ospeed

    .bss
    .align 4
counts:
    .space 256            @ 1 byte counter per ASCII value
chbuf:
    .space 1              @ single-byte read buffer
outbuf:
    .space 32             @ output buffer (we use only first 2 bytes)

    .text
    .align 4
    .arm
    .global main
    .type main, %function

    .extern open
    .extern read
    .extern write
    .extern close
    .extern tcgetattr
    .extern tcsetattr
    .extern cfmakeraw
    .extern tcflush
    .extern memset

main:
    push {r4-r11, lr}

    @ Open serial device: fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY, 0)
    ldr r0, =dev_name
    mov r1, #2            @ O_RDWR
    orr r1, r1, #256      @ O_NOCTTY
    mov r2, #0
    bl open
    mov r4, r0            @ r4 = fd
    cmp r4, #0
    blt fail

    @ Configure serial to raw mode and flush.
    mov r0, r4
    ldr r1, =options
    bl tcgetattr

    ldr r0, =options
    bl cfmakeraw

    mov r0, r4
    mov r1, #0            @ TCSANOW
    ldr r2, =options
    bl tcsetattr

    mov r0, r4
    mov r1, #2            @ TCIOFLUSH
    bl tcflush

loop_request:
    @ Reset counters for a new request line.
    ldr r0, =counts
    mov r1, #0
    mov r2, #256
    bl memset

    mov r5, #0            @ maxcount
    mov r6, #255          @ best_char (tie breaks toward smaller ASCII)
    mov r7, #0            @ bytes_received (we count up to 64)

read_loop:
    @ Read one byte (blocking) until we get exactly 1 byte.
    mov r0, r4
    ldr r1, =chbuf
    mov r2, #1
    bl read
    cmp r0, #1
    bne read_loop

    @ Load the received byte into r0.
    ldr r1, =chbuf
    ldrb r0, [r1]

    @ Newline terminates the line.
    cmp r0, #'\n'
    beq done_line

    @ Enforce MAX_IN=64: ignore extra bytes until newline arrives.
    cmp r7, #64
    bge read_loop
    add r7, r7, #1

    @ Ignore spaces (do not count them).
    cmp r0, #' '
    beq read_loop

    @ counts[c]++
    ldr r1, =counts
    add r1, r1, r0
    ldrb r2, [r1]
    add r2, r2, #1
    strb r2, [r1]

    @ Update best: higher frequency wins; ties pick smaller ASCII.
    cmp r2, r5
    bgt set_best
    beq tie_check
    b read_loop

set_best:
    mov r5, r2
    mov r6, r0
    b read_loop

tie_check:
    cmp r0, r6
    blo tie_update
    b read_loop

tie_update:
    mov r6, r0
    b read_loop

done_line:
    @ If no non-space chars were counted, return '?' and count 0.
    cmp r5, #0
    bne reply_binary
    mov r6, #'?'

reply_binary:
    @ Efficient reply (2 bytes total):
    @   outbuf[0] = best_char
    @   outbuf[1] = count
    ldr r0, =outbuf
    strb r6, [r0]
    strb r5, [r0, #1]

    mov r0, r4            @ fd
    ldr r1, =outbuf
    mov r2, #2            @ always write exactly 2 bytes
    bl write

    b loop_request

fail:
    @ Non-zero exit code on failure.
    mov r0, #1
    pop {r4-r11, pc}
