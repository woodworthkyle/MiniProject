;**************************************************************
;* This stationery serves as the framework for a              *
;* user application. For a more comprehensive program that    *
;* demonstrates the more advanced functionality of this       *
;* processor, please see the demonstration applications       *
;* located in the examples subdirectory of the                *
;* Freescale CodeWarrior for the HC12 Program directory       *
;**************************************************************



; Include derivative-specific definitions 
		INCLUDE 'derivative.inc'
CBEIF EQU $80
CCIF EQU $40
FCMDASM EQU $106  		
                  xdef DoOnStack
             

   EVEN
 DoOnStack:
    pshb ;save B - PPAGE
    ldx #SpSubEnd-2 ;point at last word to move to stack
SpmoveLoop: ldd 2,x- ;read from flash
    pshd ;move onto stack
    cpx #SpSub-2 ;past end?
    bne SpmoveLoop ;loop till whole sub on stack
    tfr sp,x ;point to sub on stack
    ldaa #CBEIF ;preload mask to register command
    call 0,x,00 ;execute the sub on the stack
    leas SpSubEnd-SpSub,sp ;de-allocate space used by sub
    pulb ;restore B
    leas -1,SP
    rtc ;to flash where DoOnStack was called
SpSub:
    ldab SpSubEnd-SpSub+2,sp ;get PPAGE back
    stab PPAGE ;Store the PPAGE address
    tfr ccr,b ;get copy of ccr
    orcc #$10 ;disable interrupts
    staa FSTAT ;[PwO] register command
    nop ;[O] wait min 4~ from w cycle to r
    nop ;[O]
    nop ;[O]
    brclr FSTAT,CCIF,* ;[rfPPP] wait for queued commands to finish  
    tfr b,ccr ;restore ccr and int condition
    rtc ;back into DoOnStack in flash
SpSubEnd: