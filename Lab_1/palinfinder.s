// palin_finder.s, provided with Lab1 in TDT4258 autumn 2024
.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	//mov r0, #1
	
check_input:
	// You could use this symbol to check for your input length
	// you can assume that your input string is at least 2 characters 
	// long and ends with a null byte
	
	// laster adressen til input inn i register r1
	ldr r0, =input // loading the address of the input string into r0
	// finding the length of the input by iterating byte for byte until we reach null character
	mov r1, #0 // setting r1 to 0, and using it as length
	loop:
	add r2, r0, r1 // load the address for the current character we want to check to r2
	ldrb r2, [r2] // load the character at the address in r2 into r2
	add r1, r1, #1 // increment the length counter r1
	cmp r2, #0 // compare the loaded character with null character
	bne loop // if not equal, we keep iterating
	sub r1, r1, #2 // if equal, we subtract 2 in order to get a value
	               // we can use to find address of last sign

	// the address of the first character is now in r0
	add r1, r0, r1 // put the address of the last character into r1
check_palindrome:
	// here we first iterate from the beginning until we find a non-space character
	b find_beginning_char // first we skip the increment, since there might be
						// a non-space character in the beginning
	skip_beginning_space:
	add r0, r0, #1 // incrementing the beginning address if we find a space
	find_beginning_char:
	ldrb r2, [r0] // loading the character at address r0 into r2
	cmp r2, #' ' // comparing this character with the space character
	beq skip_beginning_space // if its a space, we branch to skip to next character
	// now, the first non-space character is stored in r2
	
	// here we iterate from the end until we find a non-space character
	b find_end_char // first we skip the increment, since there might be
					// a non-space character in the end
	skip_end_space:
	sub r1, r1, #1 // decrementing the end address if we find a space
	find_end_char:
	ldrb r3, [r1] // loading the character at address r1 into r3
	cmp r3, #' ' // comparing this character with the space character
	beq skip_end_space // if its a space, we branch to skip to next character
	// now, the last non-space character is stored in r3
	
	cmp r0, r1 // we check if the address in r0 has gone past the address in r1
	bge is_palindrome // if true, it has succeded and the input is a palindrome

	add r0, r0, #1 // incrementing the address to point to next beginning character
	sub r1, r1, #1 // decrementing the address to point to next end character
	
	cmp r2, r3 // comparing the current beginning- and end characters
	beq check_palindrome // if they are equal, we continue looping and checking next characters
	cmp r2, #0x3F // comparing r2 to a questionmark
	beq check_palindrome // if equal, we continue looping since ? is a joker character
	cmp r3, #0x3F // comparing r3 to a questionmark
	beq check_palindrome // if equal, we continue looping
	
	// want the check to be case insensitive, so need to compare with both characters
	add r3, r3, #0x20 // first add 20 to end character
	// if r3 was originally big, by adding 20, r3 now contains the small version
	cmp r2, r3 // comparing r2 to this new character value
	beq check_palindrome // if equal, we continue checking the next characters
	sub r3, r3, #0x40 // then subtract 40 from end character
	// if r3 was originally small, by now subtracting 40 after adding 20,
	// r3 now contains the big version
	cmp r2, r3 // comparing r2 to this new character value
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
	ldr r2, =IS_PALINDROME_STRING // loading the address of the string to be printed to r2
	b write_to_jtag_uart // branch to section to write to UART
	
	
is_no_palindrome:
	// if word is not a palindrome, we turn on the 5 leftmost LEDs
	// and write 'not a palindrome' to UART
	ldr r0, =0xFF200000  // first loading the LED base address into r0
	ldr r1, [r0]         // then we load the value at the address into r1
	mov r1, #0x3E0       // writing 0x3E0 into r1, which corresponds to the leftmost
						// bits turning 1
	str r1, [r0]         // storing the modified value back to the address
	ldr r2, =NOT_PALINDROME_STRING // loading the address of the string to be printed to r2
	b write_to_jtag_uart // branch to section to write to UART
	
write_to_jtag_uart:
	ldr r1, =0xFF201000 // loading the base address of the UART into r1
	uart_loop:
	ldrb r0, [r2] // loading one byte of the message to r0
	cmp r0, #0 // comparing the character with null-character
	beq _exit // if equal, we have reached the end, and exit the program
	str r0, [r1] // store the character to the UART address
	add r2, r2, #1 // increment address to point to next character
	b uart_loop // keep iterating to check next character
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
	IS_PALINDROME_STRING: .asciz "Palindrome detected"
	NOT_PALINDROME_STRING: .asciz "Not a palindrome"
.end