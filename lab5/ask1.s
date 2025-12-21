.section .data
    buffer: .space 32
    buffer2: .space 1
    string: .asciz "Sequence of 32 chars:\n"
    string2: .asciz "\n"

.section .text
.global main


_start:       
main:        
    MOV r9, #0    

    MOV r0, #1          @ stdout
    LDR r1, =string     @ buffer address
    MOV r2, #22         @ number of bytes to write
    MOV r7, #4          @ syscall: write
    SVC 0

    MOV r0, #0          @ stdin 
    LDR r1, =buffer     @ buffer address
    MOV r2, #32         @ number of bytes
    MOV r7, #3          @ syscall: read
    SVC 0

    MOV r8, #0
    MOV r4, r0          @ save number of bytes read
    BL Read_and_Convert

    CMP r8, #27        @ Check if we need to end program
    BEQ exit

    
    MOV r0, #1          @ stdout
    LDR r1, =buffer     @ buffer address
    MOV r2, r4          @ number of bytes to write
    MOV r7, #4          @ syscall: write
    SVC 0

    CMP r4, #32         @ If less than 32 read, skip flushing
    BLT main

    
    CMP r9, #22 
    BEQ main

    @ This new line is needed for flush to know were to stop
    MOV r0, #1          @ stdout
    LDR r1, =string2    @ buffer address
    MOV r2, #1          @ number of bytes to write
    MOV r7, #4          @ syscall: write
    SVC 0

    BL Flush_stdin

    b main
exit:
    MOV r0, #0
    MOV r7, #1          @ syscall: exit
    SVC 0
    

Read_and_Convert:

    MOV r5, r4         @ Counter for characters read
    LDR r6, =buffer
    CMP r4, #2         @ Check if one character was read + \n
    BEQ Check_Done

loop:

    LDRB r0, [r6]      @ Load first byte from buffer
    CMP r0, #'\n'      @ Check for newline
    BNE Check1
    ADD r9, r9, #22
    B end

Check1:

    CMP r0, #'A'        @ Compare with 'A'
    BLT skip2           @ If less than 'A', then only care if it's a digit
    CMP r0, #'Z'        @ Compare with 'Z'
    ADDLE r0, r0, #32   @ Convert to lowercase
    BGT skip1 
    B skip3

skip1:

    CMP r0, #'a'        @ Compare with 'a'
    BLT skip3
    CMP r0, #'z'        @ Compare with 'z'
    SUBLE r0, r0, #32
    B skip3

skip2:

    CMP r0, #'0'        @ Compare with '0'
    BLT skip3
    CMP r0, #'9'        @ Compare with '9'
    BGT skip3
    ADD r0, r0, #5 
    CMP r0, #'9'        @ Do +5 and if more than '9' for the - 10
    SUBGT r0, r0, #10

skip3:

    STRB r0, [r6], #1   @ store back at the same address and increment pointer
    SUB r5, r5, #1      @ Decrease counter
    CMP r5, #0
    BNE  loop
    CMP r0, #'\n'
    ADDEQ r9, r9, #22
    B end

Check_Done:

    LDRB r0, [r6]      @ Load first byte from buffer
    MOV r8, #27        @ So that main ends program
    CMP r0, #'q'
    BEQ end 
    CMP r0, #'Q'
    BEQ end
    MOV r8, #0
    B Check1
    
end:

    BX lr

Flush_stdin:          @ Clears chars after 32 have been read

    PUSH {lr}

Flush_loop:

    MOV r0, #0          @ stdin 
    LDR r1, =buffer2    @ buffer address
    MOV r2, #1         @ number of bytes
    MOV r7, #3          @ syscall: read
    SVC 0

    CMP r0, #0 
    BEQ Flush_done

    LDRB r0, [r1]
    CMP r0, #'\n'         @ Check for newline
    BNE Flush_loop

Flush_done:
    POP {lr}
    BX lr
