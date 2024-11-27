// CPUlator "cheat-sheet", an appendix for your help at TDT4258-exam November 2024, V1.0, 2024-10-28.
//***********************************************************************************************************
// Registers: R0, R1 ... R12. R13 = SP, R14 = LR, R15 = PC    // Comments should be e-mailed to Lasse Natvig.
// Register convention on function calls: Parameters passed in R0..R3, additional parameters passed via stack 
// Values are returned in R0..R1. R0..R3 are caller saved registers, and R4..R12 are callee saved registers
.global _start  // SP = Stack Pointer, LR = Link Register, PC = Program Counter
_start: // Note: this program "does nothing" and is by will made rather compact to fit in one page.
.equ VGAaddress, 0xc8000000 // .equ directive defining a readable name for a given memory address. 
    MOV R0, #15  // MOVe immediate value 15 (decimal) to register R0
	MOV R0, #5555 // will be translated to MOVW (MOV word) since immediate const is large
	MOV R8, R0   // MOVe (ie. copy) value of R0 into R8. Note first register is destination register
	LDR R1, =a   // LoaD address of a into Register R1 (Corresponds to "ADR R1, a" in the textbook)
	LDR R2, [R1] // LDR, load what R1 points to into R2
	LDR R3, [R1, #4] // LDR, load what R1 points to + offsett 4 bytes
	ADD R4, R2, R3 // ADDs R2 and R3 and stores result in R4. ADD with only two operands is possible
	ADD R4, R4, #1 // ADDs immediate value 1 (decimal) to R4. 
	STR R5, [R1]  // SToRe contents of R5 into location given by R1, offset can be used like for LDR above
	B proceed // Branch unconditionally to label   // STR (line above), Note first register is source
	B . // Branch unconditionally to itself (.), gives endless loop. This instruction is not executed
proceed: // label // The second operand in a compare instruction can be an immediate value like CMP, R2, #10
	CMP R2, R3 // CoMPare. Computes R2 - R3, sets status bits according to result, throws the result
	BLT nice // Branch Less Than, branch to the label nice if R2 was less than R3. BEQ is Branch Equal
	         // BGE greater or equal, BNE Branch Not Equal, and BGT Branch Greater Than  
	ADD R4, R4, #1 // This instruction will not be executed, BLT nice will jump over
nice: SUB R4, R3, #18 // SUBtracts 18 (decimal) from R3 and stores in R4, does not update status register (cpsr)
	SUBS R4, R3, #18 // Same as above but updates Status register (cpsr). Similar xxxS for some other instructions
	LSL R2, R4, #2 //Logical Shift Left R4 with 2 bit positions, assign to R2
	PUSH {R1} // PUSH and POP operates on the stack
	PUSH {R2}  // it is a LIFO structure (Last In First Out)
	LDR R9, [SP, #4] // Load from stack with offset 4 to SP (one word below top) into R9 (ie. value of R1)
	POP {R1}
	POP {R2} // values of R1 and R2 are now swapped after execution of this instruction
	PUSH {R1-R4} // NOTE executed in this order  PUSH {R4}, PUSH {R3}, PUSH {R2}, PUSH {R1} 
	MOV R2, #1234 
	POP {R1-R4}  // R1-R4 are now restored from the state, check that R2 now has the same value as 2 instr. above
	LDR R0, =0xFFFF0000
	LDR R1, =0x0000FFFF
	AND R3, R0, R1 // Bit-wise AND of R0 and R1 stored in R3. ANDS will also Set status register (See SUBS above)
	ORR R3, R0, R1 // Bit-wise OR of R0 and R1 stored in R3
	LSR R0, #1  // Logical Shift Right 1 bit position of R0
	EOR R3, R0, R1 // Bit-wise exclusive OR of R0 and R1 into R3
	LDR R5, =VGAaddress  // VGAaddress is defined as 0xc8000000 with .equ directive close to the top
	LDR R4, =#0x00000f0f	// Green in VGA
	MOV R3, #0
	STRH R4, [R5, R3]  // Store half-word of R4, lower 16 bits at address R5 + offset R3
	LDR R2, =Hello
	LDRB R0, [R2], #1 // LoaD into Register a Byte pointed to by R2. Increment R2
	PUSH {R2} // R2 is caller-saved, and used in PUT_JTAG. Will be changed by PUT_JTAG
	BL PUT_JTAG // call to function with Branch and Link, sets link register LR. Prints 'H'
	POP {R2} // Restore R2
	LDRB R0, [R2] // LoaD into Register a Byte pointed to by R2.
	BL PUT_JTAG // watch that R2 is NOT incremented here Print 'e'
	AND R5, R4, #0x00FF // Picks the lower byte of R4 and stores in R5 by masking 
	MUL R0, R4, R5 // MULiply R4 and R5 and store in R0
	MLA R0, R4, R5, R9 // MuLtiply Accumulate, here R0 is set to the value of (R4 * R5) + R9
	BKPT #0 // Breakpoint, halts the program. Saves energy compared to spinning in a loop with "B ."
PUT_JTAG: // input param char in R0, assumes call by BL
	LDR R1, =0xFF201000 // JTAG UART base address, assigned to R1
	LDR R2, [R1, #4] // read the JTAG UART control register into R2
	LDR R3, =0xFFFF0000 // mask, top 16 bits of control register holds write-space
	ANDS R2, R2, R3 // Logical AND, R2 becomes zero if no space available, sets status register
	BEQ PUT_JTAG // if no space, try again (Busy-waiting loop)
	STR R0, [R1] // send the character by writing to UART data register
	BX LR  // return from function using the Link Register LR
.section .data
a: // array a[0], a[1], a[2]
	.word 10 // decimal values
	.word 20 // word is 4 bytes
	.word 30
Hello: .asciz "Hello "