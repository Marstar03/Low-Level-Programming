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

_start: // some code demonstrating SetPixel (x, y, c)
//    LDR R3, =0x0000ffff	// White
    LDR R3, =0x000000ff	// Blue
	MOV R4, #0 // R4 stores x-coordinate
	MOV R5, #0 // R5 stores y-coordinate
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL SetPixel
//    LDR R3, =0x00000000	// Black
    LDR R3, =0x00000f0f	// Green
	ADD R4, R4, #1
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL SetPixel
    LDR R3, =0x0000ff00	// yellow
	MOV R4, #0
	ADD R5, R5, #1
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL SetPixel
    LDR R3, =0x0000f0f0	// red
	ADD R4, R4, #1
	MOV R1, R4 // prep param passing
	MOV R2, R5 // for x and y 
	BL SetPixel
	B .