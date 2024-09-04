// palin_finder.s, provided with Lab1 in TDT4258 autumn 2024
.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	mov r0, #1
	//mov r7, #0xFF200000
	b check_input
	b _exit

	
check_input:
	// You could use this symbol to check for your input length
	// you can assume that your input string is at least 2 characters 
	// long and ends with a null byte
	
	// laster adressen til input inn i register r1
	ldr r1, =input
	// finner lengden til input ved å iterere gjennom helt til vi når en null-character
	mov r2, #0
	loop:
	add r3, r1, r2
	ldrb r4, [r3]
	add r2, r2, #1
	cmp r4, #0
	bne loop
	sub r2, r2, #2
	


	mov r4, r1
	add r5, r1, r2
	// nå inneholder r4 adressen til fremste tegn og r5 adressen til bakerste tegn
check_palindrome:
	b find_beginning_char
	skip_beginning_space:
	add r4, r4, #1
	find_beginning_char:
	ldrb r3, [r4]
	//add r4, r4, #1
	cmp r3, #' '
	beq skip_beginning_space
	// Nå ligger første ikke-mellomrom tegn i r3
	
	b find_end_char
	skip_end_space:
	sub r5, r5, #1
	find_end_char:
	ldrb r6, [r5]
	// r6 inneholder nå bakerste tegn
	//sub r5, r5, #1
	cmp r6, #' '
	beq skip_end_space
	// nå ligger bakerste ikke-mellomrom tegn i r6
	
	// Sjekker om adressen i r4 har gått forbi adressen i r5
	// Isåfall har det succeded, og input er et palindrom
	cmp r4, r5
	bge is_palindrome

	add r4, r4, #1
	sub r5, r5, #1
	
	cmp r3, r6
	beq check_palindrome
	cmp r3, #0x3F
	beq check_palindrome
	cmp r6, #0x3F
	beq check_palindrome
	// Sammenligner med store bokstaver
	// Sammenligner først med r6 + 20 i tilfelle r6 er stor
	add r6, r6, #0x20
	cmp r3, r6
	beq check_palindrome
	// Sammenligner deretter med (gammel) r6 - 20 i tilfelle r6 er liten
	sub r6, r6, #0x40
	cmp r3, r6
	beq check_palindrome
	
	b is_no_palindrome
	
	
is_palindrome:
	// Switch on only the 5 rightmost LEDs
	// Write 'Palindrom detected' to UART
	ldr r0, =0xFF200000  // Load the address into r0
	ldr r1, [r0]         // Load the value at the address into r1
	mov r1, #0x1F        // Set the first bit of r1
	str r1, [r0]         // Store the modified value back to the address
	ldr r4, =IS_PALINDROME_STRING
	b write_to_jtag_uart
	
	
is_no_palindrome:
	// Switch on only the 5 leftmost LEDs
	// Write 'Not a palindrom' to UART
	ldr r0, =0xFF200000  // Load the address into r0
	ldr r1, [r0]         // Load the value at the address into r1
	mov r1, #0x3E0       // Set the first bit of r1
	str r1, [r0]         // Store the modified value back to the address
	ldr r4, =NOT_PALINDROME_STRING
	b write_to_jtag_uart
	
write_to_jtag_uart:
	//MOV SP, #DDR_END - 3 // highest memory word address
	/* print a text string */
	LOOP:
	ldrb R0, [R4]
	cmp R0, #0
	beq CONT // string is null-terminated
	bl PUT_JTAG // send the character in R0 to UART
	add R4, R4, #1
	b LOOP
	/* read and echo characters */
	CONT:
	bl GET_JTAG // read from the JTAG UART
	cmp R0, #0 // check if a character was read
	beq CONT
	bl PUT_JTAG
	
	.global PUT_JTAG
	PUT_JTAG:
	//LDR R1, =JTAG_UART_BASE // JTAG UART base address
	ldr R1, =0xFF201000
	ldr R2, [R1, #4] // read the JTAG UART control register
	ldr R3, =0xFFFF0000
	ands R2, R2, R3 // check for write space
	beq END_PUT // if no space, ignore the character
	str R0, [R1] // send the character
	END_PUT:
	bx LR
	
	.global GET_JTAG
	GET_JTAG:
	//LDR R1, =JTAG_UART_BASE // JTAG UART base address
	ldr R1, =0xFF201000
	ldr R0, [R1] // read the JTAG UART data register
	ands R2, R0, #0x8000 // check if there is new data
	beq RET_NULL // if no data, return 0
	and R0, R0, #0x00FF // return the character
	b END_GET
	RET_NULL:
	mov R0, #0
	END_GET:
	bx LR
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "hei paap ieh"
	IS_PALINDROME_STRING: .asciz "Palindrome detected"
	NOT_PALINDROME_STRING: .asciz "Not a palindrome"
.end