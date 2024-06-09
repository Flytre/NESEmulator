    .org $8000      ; Start of the program

    LDX #$00        ; Load 0 into X register
    LDY #$10        ; Load 16 into Y register
    LDA #$FF        ; Load 255 into A register
    STA $6000       ; Store A (255) at address $6000
    INX             ; Increment X
    INY             ; Increment Y
    STX $6001       ; Store X (1) at address $6001
    STY $6002       ; Store Y (17) at address $6002

    LDA $6000       ; Load value at address $6000 into A
    BEQ DONE        ; Branch to DONE if A is zero (it isn't, so this will be skipped)
    LDA #$00        ; Load 0 into A
    STA $6003       ; Store A (0) at address $6003

DONE:
    BRK             ; Break - end of program

    .org $FFFC
    .word $8000     ; Reset vector
    .word $FFFE     ; IRQ/BRK vector
    .word $FFEA     ; NMI vector