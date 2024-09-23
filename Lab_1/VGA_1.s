// VGAmini.s provided with Lab1 in TDT4258 autumn 2024
// 320 (width) x 240 (height) pixels, 
// 2 bytes per pixel RGB format, 1024 bytes is used to store one row
.global _start
.equ VGAaddress, 0xc8000000 // Memory storing pixels

SetPixel: // assumes R1 = x-coord, R2 = y, R3 = colorvalue
    LDR R0, =VGAaddress
    LSL R2, R2, #10 // y-address shift left 10 bits (1024 bytes)
    LSL R1, R1, #1  // x-adress  shift left 1 bit (2 bytes)
    ADD R2, R1 // R2 is now correct offset
    STRH R3, [R0,R2]  // Store half-word, lower 16 bits at address R0 + offset R2
    BX LR
	
BigPixel:
	mov r6, #10
	mul r4, r4, r6
	mul r5, r5, r6
	mov r7, #0
	mov r8, #0
	loop:
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL SetPixel
	add r4, r4, #1
	add r7, r7, #1
	cmp r7, #10
	blt loop
	sub r4, r4, #10
	mov r7, #0
	add r5, r5, #1
	add r8, r8, #1
	cmp r8, #10
	blt loop
	b return

_start: // some code demonstrating SetPixel (x, y, c)
    LDR R3, =0x0000ffff	// White
	MOV R4, #0 // R4 stores x-coordinate
	MOV R5, #0 // R5 stores y-coordinate
	//MOV R1, R4 // prep param passing
	//MOV R2, R5 // for x and y 
	b BigPixel
return:
	B .