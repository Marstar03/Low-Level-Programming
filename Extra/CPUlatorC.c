// TDT4258 autumn 2024, CPUlatorC.c --- code to test debugging example for C-lectures, Lasse Natvig.
// Version 1.1.0, 2024-09-20, LN
//
// Comments or questions are always welcome. Please write in e-mail to Lasse.
// ***********************************************************************************************
// demonstrates conditional compilation, logging, and UART output

// Can optionally use UART for feedback by defining CPULATOR_UART
// demonstrates multi-version code via #define/#ifdef/#endif, and conditional logging. This is handled by the C preprocessor.
 
#include <stdio.h> // Include the stdio.h header file for standard input/output.
// We do not know yet how much of C standard library we have access to under CPUlator, don't assume too much
#include <stdbool.h> // Include the stdbool.h header file for boolean type and constants, not needed with C23

#define LOGGING // used to turn on/off the logging of function calls

const int VGA_MAX_X = 320; // Constants in C are defined with the const keyword, and are uppercase by convention
const int VGA_MAX_Y = 240; 

//#define CPULATOR_UART  // Comment out this on other systems than CPUlator
#ifdef CPULATOR_UART 
	void writeUart(char c);
	asm("writeUart: \n\t" // This is the very specific format needed for inline assembly in the CPUlator system
		"// Code from PUT_JTAG in lectured example CPUlator3.s, // input param char in R0, assumes call by BL \n\t"
		"LDR R1, =0xFF201000 // JTAG UART base address \n\t"
		"LDR R2, [R1, #4] // read the JTAG UART control register \n\t"
		"LDR R3, =0xFFFF0000 // mask, top 16 bits holds write-space \n\t"
		"ANDS R2, R2, R3 // Logical and, R2 becomes zero if no space available \n\t"
		"BEQ writeUart // if no space, try again (Busy-waiting loop) \n\t"
		"STR R0, [R1] // send the character \n\t"
		"BX LR \n\t");
#else
	void writeUart(char c) { printf("%c", c);}  // The same functionality as the UART, but using the console
#endif
//*****************end of conditional compilation for CPULATOR_UART *****************
 
//******************************************************************************
// These routines are used to ease printing in the UART window 
void writeString(char* str) {  
	while (*str != '\0') { writeUart(*str++); }
}
void newline() {writeUart('\n');}
void writeInt(int num) { // from github copilot
    char buffer[20]; // assuming the maximum number of digits is 20
    int i = 0;
    if (num == 0) { // Handle the case when num is 0 separately
        writeUart('0');
        return;
    }
    if (num < 0) { // Handle negative numbers
        writeUart('-');
        num = -num;
    }
    while (num > 0) { // Convert the number to string in reverse order
        buffer[i++] = num % 10 + '0'; // % is the modulo operator, '0' is the ASCII value of 0
        num /= 10; // / is the integer division operator, and /= is the compound assignment operator
    }
    for (int j = i - 1; j >= 0; j--) { // Print the digits in correct order
        writeUart(buffer[j]);
    }
}

typedef struct{
    int x;
    int y;
} Point;
typedef struct {
    Point* p;
    int width;
    int height;   
} Rectangle;

#ifdef LOGGING // In case we have little memory, we should not compile this code when we don't use logging
    void loggRectangle(Rectangle* r, char *msg) {
            writeString(msg);
            writeInt(r->p->x); 
            writeString(", ");
            writeInt(r->p->y);
            writeString(" width: ");
            writeInt(r->width);
            writeString(" height: ");
            writeInt(r->height);
            newline();
    }
#endif

bool DrawRectangle(Rectangle* r) {
    if (((r->p->x < 0) || ((r->p->x + r->width) > 320)) 
       || ((r->p->y < 0)) || ((r->p->y + r->height) > 240)) {
        #ifdef LOGGING
            loggRectangle(r, "Rectangle outside VGA at ");
        #endif
        return false;
    }
    // Inside the VGA area, draw the rectangle
    // TODO do the VGA graphics here 
    // ...
    #ifdef LOGGING
        loggRectangle(r, "Drawing rectangle at ");
    #endif
    return true;
}

int main(int argc, char *argv[]) {
    printf("*** Welcome to CPUlatorC.c example\n"); // output to console
    printf("VGA size is %d(x) x %d(y)\n\n", VGA_MAX_X, VGA_MAX_Y);

    Point p1 = {10, 20}; // create two points and two rectangles
    Point p2 = {100, 100};
    Rectangle r1 = {&p1, 30, 40};
    Rectangle r2 = {&p2, 50, 60};

    bool inside = true;
    int iterations = 0;
    while (inside) { // draw rectangles until one is outside the VGA area
        iterations++;   // move the rectangles to test the inside - outside logic
        inside = true;
        inside = DrawRectangle(&r1) && DrawRectangle(&r2); // && is the logical AND operator.  
        r1.p->x += 100; //   a += b  means a = a + b, and is the compound assignment operator
        r2.p->x += 50;  // moves rectangles to test inside - outside logic (It is far too little testing)
    }
    writeString("The loop iterated "); 
    writeInt(iterations); 
    writeString(" times before one of the rectangles was outside the VGA area\n");
    return 0;
}