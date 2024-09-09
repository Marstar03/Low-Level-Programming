// palin_finder.s, provided with Lab1 in TDT4258 autumn 2024
.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	mov r0, #1
	
check_input:
	// You could use this symbol to check for your input length
	// you can assume that your input string is at least 2 characters 
	// long and ends with a null byte
	
	// laster adressen til input inn i register r1
	ldr r1, =input // loading the address of the input string into r1
	// finding the length of the input by iterating byte for byte until we reach null character
	mov r2, #0 // setting r2 to 0, and use it as length
	loop:
	add r3, r1, r2 // load the address for the current character we want to check to r3
	ldrb r4, [r3] // load the character at the address in r3 into r4
	add r2, r2, #1 // increment the length counter r2
	cmp r4, #0 // compare the loaded character with null character
	bne loop // if not equal, we keep iterating
	sub r2, r2, #2 // if equal, we subtract 2 in order to get a value
	               // we can use to find address of last sign

	mov r4, r1 // put the address of the first character into r4
	add r5, r1, r2 // put the address of the last character into r5
check_palindrome:
	// here we first iterate from the beginning until we find a non-space character
	b find_beginning_char // first we skip the increment, since there might be
						// a non-space character in the beginning
	skip_beginning_space:
	add r4, r4, #1 // incrementing the beginning address if we find a space
	find_beginning_char:
	ldrb r3, [r4] // loading the character at address r4 into r3
	cmp r3, #' ' // comparing this character with the space character
	beq skip_beginning_space // if its a space, we branch to skip to next character
	// now, the first non-space character is stored in r3
	
	// here we iterate from the end until we find a non-space character
	b find_end_char // first we skip the increment, since there might be
					// a non-space character in the end
	skip_end_space:
	sub r5, r5, #1 // decrementing the end address if we find a space
	find_end_char:
	ldrb r6, [r5] // loading the character at address r5 into r6
	cmp r6, #' ' // comparing this character with the space character
	beq skip_end_space // if its a space, we branch to skip to next character
	// now, the last non-space character is stored in r6
	
	cmp r4, r5 // we check if the address in r4 has gone past the address in r5
	bge is_palindrome // if true, it has succeded and the input is a palindrome

	add r4, r4, #1 // incrementing the address to point to next beginning character
	sub r5, r5, #1 // decrementing the address to point to next end character
	
	cmp r3, r6 // comparing the current beginning- and end characters
	beq check_palindrome // if they are equal, we continue looping and checking next characters
	cmp r3, #0x3F // comparing r3 to a questionmark
	beq check_palindrome // if equal, we continue looping since ? is a joker character
	cmp r6, #0x3F // comparing r6 to a questionmark
	beq check_palindrome // if equal, we continue looping
	
	// want the check to be case insensitive, so need to compare with both characters
	add r6, r6, #0x20 // first add 20 to end character
	// if r6 was originally big, by adding 20, r6 now contains the small version
	cmp r3, r6 // comparing r3 to this new character value
	beq check_palindrome // if equal, we continue checking the next characters
	sub r6, r6, #0x40 // then subtract 40 from end character
	// if r6 was originally small, by now subtracting 40 after adding 20,
	// r6 now contains the big version
	cmp r3, r6 // comparing r3 to this new character value
	beq check_palindrome // if equal, we continue checking the next characters
	
	b is_no_palindrome // if none of the checks were successful, the characters are
						// not the same, and the word is therefore not a palindrome
	
	
is_palindrome:
	// if word is a palindrome, we turn on the 5 rightmost LEDs
	// and write 'palindrome detected' to UART
	ldr r0, =0xFF200000  // first loading the LED base address into r0
	ldr r1, [r0]         // then we load the value at the address into r1
	mov r1, #0x1F        // writing 0x1F into r1, which corresponds to the rightmost
						// bits turning 1
	str r1, [r0]         // storing the modified value back to the address
	ldr r4, =IS_PALINDROME_STRING // loading the address of the string to be printed to r4
	b write_to_jtag_uart // branch to section to write to UART
	
	
is_no_palindrome:
	// if word is not a palindrome, we turn on the 5 leftmost LEDs
	// and write 'not a palindrome' to UART
	ldr r0, =0xFF200000  // first loading the LED base address into r0
	ldr r1, [r0]         // then we load the value at the address into r1
	mov r1, #0x3E0       // writing 0x3E0 into r1, which corresponds to the leftmost
						// bits turning 1
	str r1, [r0]         // storing the modified value back to the address
	ldr r4, =NOT_PALINDROME_STRING // loading the address of the string to be printed to r4
	b write_to_jtag_uart // branch to section to write to UART
	
write_to_jtag_uart:
	ldr r1, =0xFF201000 // loading the base address of the UART into r1
	uart_loop:
	ldrb R0, [R4] // loading one byte of the message to r0
	cmp R0, #0 // comparing the character with null-character
	beq _exit // if equal, we have reached the end, and exit the program
	str r0, [r1] // store the character to the UART address
	add R4, R4, #1 // increment address to point to next character
	b uart_loop // keep iterating to check next character
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "hei pabp ieh"
	IS_PALINDROME_STRING: .asciz "Palindrome detected"
	NOT_PALINDROME_STRING: .asciz "Not a palindrome"
.end