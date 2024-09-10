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
	LDR R0, =VGAaddress
	mov r6, #10
	mul r2, r2, r6
	mul r1, r1, r6
	mov r7, #0
	mov r8, #0
	loop:
	//MOV R1, R4 // prep param passing
	//MOV R2, R5 // for x and y 
	BL SetPixel
	add r1, r1, #1
	add r7, r7, #1
	cmp r7, #10
	blt loop
	sub r1, r1, #10
	mov r7, #0
	add r2, r2, #1
	add r8, r8, #1
	cmp r8, #10
	blt loop

_start: // some code demonstrating SetPixel (x, y, c)
    LDR R3, =0x0000ffff	// White
    //LDR R3, =0x000000ff	// Blue
	MOV R4, #0 // R4 stores x-coordinate
	MOV R5, #0 // R5 stores y-coordinate
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL BigPixel
	B .