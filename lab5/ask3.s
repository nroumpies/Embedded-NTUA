.text
.align 4
.type strlen, %function
.global strlen

strlen:
    MOV r1, r0          @ Convention to pass args in r0-r3 
loop:
    LDRB r2, [r1], #1    @ Load byte per byte
    CMP r2, #0           
    BNE loop             @ Repeat until NULL
    SUB r0, r1, r0       @ Calc Steps, r1 is out
    SUB r0, r0, #1       @ Adjust for null terminator
    BX lr                @ Return

.text
.align 4
.type strcpy, %function
.global strcpy

strcpy:
    MOV r3, r0           @ Save dest, since conventionally out is in r0
loop2:
    LDRB r2, [r1], #1    @ Load byte and increment
    STRB r2, [r3], #1    @ Store byte and increment
    CMP r2, #0           
    BNE loop2            @ Repeat until NULL
    BX lr                @ Return

.text
.align 4
.type strcat, %function
.global strcat
strcat:
    MOV r3, r0           @ Save dest, since conventionally out is in r0

find_length: @ Could use strlen, but BL adds overhead and impacts performance. But good for cache locality, space and maintainability.

    LDRB r2, [r3], #1    @ Find the end of dest
    CMP r2, #0           
    BNE find_length      @ Repeat until NULL
    SUB r3, r3, #1       @ One step back to overwrite null terminator

copy_data: @ Could use strcpy, but BL adds overhead and impacts performance. But good for cache locality, space and maintainability.

    LDRB r2, [r1], #1    @ Load byte and increment
    STRB r2, [r3], #1    @ Store byte and increment
    CMP r2, #0           
    BNE copy_data        @ Repeat until NULL
    BX lr                @ Return


.text
.align 4
.type strcmp, %function
.global strcmp
strcmp3:
Compare_loop:   
    LDRB  r2, [r0], #1
    LDRB  r3, [r1], #1
    CMP   r2, r3      @ Compare byte by byte
    BNE   Out
    CMP   r2, #0
    BNE   Compare_loop
    MOV   r0, #0      @ if equal, out = 0
    BX    lr
Out:                  @ Out with not equal
    MOVHI r0, #1      @ if r2 > r3, out = 1
    MOVLO r0, #-1     @ if r2 < r3, out = -1
    BX    lr        
    
