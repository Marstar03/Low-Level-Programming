// palin_finder.s, provided with Lab1 in TDT4258 autumn 2024
.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	mov r0, #1
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
	sub r2, r2, #1
	


	mov r4, r1
	add r5, r1, r2
	// nå inneholder r4 adressen til fremste tegn og r5 adressen til bakerste tegn
check_palindrom:
	find_beginning_char:
	ldrb r3, [r4]
	add r4, r4, 1
	cmp r3, #' '
	beq find_beginning_char
	// Nå ligger første ikke-mellomrom tegn i r3
	
	find_end_char:
	ldrb r6, [r5]
	// r6 inneholder nå bakerste tegn
	sub r5, r5, 1
	cmp r6, #' '
	beq find_end_char
	// nå ligger bakerste ikke-mellomrom tegn i r6
	
	// Sjekker om adressen i r4 har gått forbi adressen i r5
	// Isåfall har det succeded, og input er et palindrom
	cmp r4, r5
	bge r4, r5, is_palindrom
	
	cmp r3, r6
	beq check_palindrome
	bne is_no_palindrome
	
// --------------------------------------
	
	// Here you could check whether input is a palindrom or not
	// r2 inneholder lengden på input i bytes
	
	// Indeks fremover
	mov r7, #0
	// Indeks bakover
	mov r8, r2
	loop2:
	// Adresse fremover
	add r3, r1, r7
	ldrb r4, [r3]
	add r7, r7, #1
	// Adresse bakover
	add r9, r1, r7
	ldrb r4, [r9]
	add r7, r7, #1
	// Nå ligger
	
	cmp r4, #0
	bne loop
	sub r2, r2, #1
	
	
	b _exit
	
	
is_palindrom:
	// Switch on only the 5 rightmost LEDs
	// Write 'Palindrom detected' to UART
	
	
is_no_palindrom:
	// Switch on only the 5 leftmost LEDs
	// Write 'Not a palindrom' to UART
	
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
.end