/* TDT4258-C-QuickRef-2024.c V1.0 //This will be made available to students as 
   a searchable (ctrl-F) PDF resource in Inspera during exam. It is executable
   so that you can study its semantics in a debugger or by adding print statements
   *before* the exam. It is deliberately made compact and is NOT a complete reference. 
   Thanks to Jan Chr Meyer (IDI) for input and discussion. 
   Last updated 2024-11-11 by Lasse Natvig.  
*/ 

// Preprocessor directives *******************************************************
#include <stdio.h>     // Standard I/O, e.g. printf
#include <stdlib.h>    // Standard library, e.g. malloc
#include <string.h>    // String manipulation, e.g. memset and more functions
#include <stdint.h>    // Integer types, e.g. int32_t
#include <stdbool.h>   // Boolean type, e.g. bool
#include <limits.h>    // Integer limits, e.g. INT_MAX

// Macro definitions: constants
#define AVOGADROS_NUMBER (6.02214076e23)
// Macro definitions: parametric
#define SQUARE(x) ((x)*(x))

// Prmitive data types **********************************************************
void primitive_types ( void ) {
    // Integers, architecture-specific (size of these types may vary between platforms)
    char a = CHAR_MAX; short b = SHRT_MAX;
    int c  = INT_MAX;  long d  = LONG_MAX;
    printf ( "%d %d %d %ld\n", a, b, c, d );  // later on, use debugger to inspect values

    // Literals
    int n  = 99;                 // Decimal
    int n2  = 0xFF;              // Hexadecimal
    long n3 = 99L;               // Long integer
    int e  = 0b10101111;         // Binary
    char f = 'A';                // Character (ASCII/byte)
    const char *h = "Hello";     // String literal
    float f1  = 0.5F;            // Single precision floating point
    double f2 = 0.5L;            // Double precision floating point
    double k = 1.25e-3;          // Scientific notation (1.25*10^-3)

    // Integers, architecture-independent (size is guaranteed)
    int8_t  byte = 1; int16_t word = 2; int32_t longword = 3; int64_t quadword = 4;
    uint8_t byte2 = 1; uint16_t word2 = 2; uint32_t longword2 = 3; uint64_t quadword2 = 4;

    // Booleans
    bool test = true;
    test = !test; // ! is logical NOT, test is now false
}

void strings(void) {
    const char *fixed_string = "Hello, const char *";  // text that never changes
    // Text generated at run time, locally scoped char array
    char mutable_string[14];
    memset(mutable_string, 0, 14*sizeof(char));  // 13 chars + zero at end
    sprintf(mutable_string, "Hello, char *");    // 13 characters
    mutable_string[0] = 'B';  // string is an array of characters
    mutable_string[5] = 'w';
    puts(mutable_string); // Print to console
    // Text generated at run time, dynamic heap allocation
    char *string_pointer = (char*) malloc(14*sizeof(char));
    memset ( string_pointer, 0, 14*sizeof(char) );  // 13 chars + zero at end
    sprintf ( mutable_string, "Hello, malloc" );    // 13 characters
    free ( string_pointer );  // Release memory
}

extern void pBin(uint8_t num); //helperfunction defined later, demonstrates forward declaration
void operators(void) {
    // Arithmetic operators
    int a = 7, b = 5;  // Comma can be used here, but is not recommended
    int c = a + b; // Sum
    c = a - b;  c = a * b;    // Difference // Product 
    c = a / b;  c = a % b;   // Quotient (integer division) // Remainder / modulus
    a++; a--; // Postincrement and postdecrement 
    ++a; --a; // Preincrement and predecrement                  

    // Relational operators and ternary operator
    bool lt, lte, gt, gte, eq, neq;
    lt  = (a < b);    // Less than
    lte = (a <= b);   // Less than or equal
    gt  = (a > b);    // Greater than
    gte = (a >= b);   // Greater than or equal
    eq  = (a == b);   // Equal
    neq = (a != b);   // Not equal
    int max = ((a > b) ? (a) : (b));  // ternary operator 

    
    a = 7, b = 5; 
    // Logical operators. Use ( ) around conditions and to avoid precedence issues. Nesting is allowed.
    if ( (a>b) && (b<a) ) puts ( "&& (and) check ok" ); // if (a>b) is false, (b<a) is not evaluated (short-circuiting)
    if ( (a>b) || (b<a) ) puts ( "|| (or)  check ok" ); // if (a>b) is true, (b<a) is not evaluated (short-circuiting)
    if ( ! (a<b) )        puts ( "!  (not) check ok" ); // ! is logical NOT

    // Bitwise operators. We use pBin here to print binary representation of numbers to help understanding
    pBin(a); fputs(" & ", stdout); pBin(b); fputs(" = ", stdout); pBin(a&b); putchar('\n');  // Bitwise and
    pBin(a); fputs(" | ", stdout); pBin(b); fputs(" = ", stdout); pBin(a|b); putchar('\n');  // Bitwise or
    pBin(a); fputs(" ^ ", stdout); pBin(b); fputs(" = ", stdout); pBin(a^b); putchar('\n');  // Bitwise xor
    putchar('~'); pBin(a); fputs(" = ", stdout); pBin(~a); putchar('\n');              // Bitwise complement
    pBin(a); fputs(" << ", stdout); printf("%d = ", b); pBin(a<<b); putchar('\n'); // << is left shift
    a = 128, b = 2;
    pBin(a); fputs(" >> ", stdout); printf("%d = ", b); pBin(a>>b); putchar('\n'); // >> is right shift

    // Compound assignment operators
    a += b; // means a = a + b. Similar for -=, *=, /=, %=, <<=, >>=, &=, ^=, |=

    // Pointers are addresses. & is the address-of operator, * is the dereference operator
    int *p = &a; // p is a pointer to an integer, initialized to the address of a
        // It is a matter of style(or taste) whether to write int* p or int *p
    *p = 42; // Dereference p to change the value of a (Write 42 to the memory location pointed to by p)   
}

void derived_types() {
    typedef double Temperature; // Type alias or type definition
    Temperature boiling_point = 100.0;
    enum Weather { sun, rain, sleet, hail, powder}; // Enumerated type
    enum Weather stydenterhytta = powder;

    /* Structured types: */
    struct { int x; int y; } point_one;  // declaration of a struct variable
    point_one.x = 7; point_one.y = 5; // Direct access
    typedef struct { int x; int y; } Point; // Definition of type for repeated use or increased readability
    Point point_two = { .x = 14, .y = 10 }; // Structured initializer
    Point myPoint = { 28, 20 }; // Equivalent notation

    // Dynamic allocation and structured assignment
    Point* point_three = malloc ( sizeof(Point) );  // sizeof() is a compile-time operator giving the size of a type in bytes   
    *point_three = (Point) { .x = 28, .y = 20 };
    printf ( "%d %d \n", (*point_three).x, point_three->y ); // Indirect access in two equivalent notations
    free ( point_three ); // Release memory
}

void arrays(void) {
    int arr[5] = {1, 2, 3, 2, 1}; // array, size known at compile time, initialization of all members
    char arr2D[3][4] = { // 2D array of chars
        {'A', 'B', 'C', 'D'},  // 3 or more dimension possible
        {'E', 'F', 'G', 'H'}, // arrays of structs, pointers and other types are possible
        {'I', 'J', 'K', 'L'} 
    };
}

void control_flow() {
    int x = 3;     // Conditionals, if-statments. The else part is optional
    if ( x == 3 ) { // You do not need the { and } around a single statement but it is strongly recommended 
        puts ( "x equals 3\n" );
    } else { puts ( "x is not equal to 3\n" );}
    switch ( x ) { // Multi-way branches
        case 0:
            puts ( "Zero" );
            break;  // If you omit break, the next case will be executed as well (fall-through)
        case 3:
            puts ( "Three" );
            break;
        default:
            puts ( "Neither zero nor three" );
            break;
    }
    // Loops with induction variable
    for ( int i=0; i<10; i++ ) { // for (initialization; condition; update)
        if ( i == 5 ) continue; // Skip the rest of the loop body. Demonstrates continue
        printf ( "%d ", i ); // Continue and break can be used in while and do-while loops as well
        if (i == 7) break; // Exit the loop. Demonstrates break
    }
    printf ( "\n" );  // \n often flushes the output buffer
    int i = 0;     // Head-controlled loop
    while ( i<10 ) {  // If i is set to 10, loop will never run
        printf ( "%d ", i ); i = i+1;
    }
    printf ( "\n" );
    i = 0; // If i is set to 10, loop will run once
    do {     // Tail-controlled loop
        printf ( "%d ", i ); i = i+1;
    } while ( i<10 );       
    printf ( "\n" );
}

int main() {
    primitive_types();
    strings();
    operators();
    derived_types();
    arrays();
    control_flow();
    return (0);
}
void pBin(uint8_t num) {  // Helper function to print binary representation of a 8 bit number
    char binary[9]; // 8 bits + null terminator
    binary[8] = '\0'; // null terminator
    for (int i = 7; i >= 0; i--) { 
        binary[i] = (num & 1) ? '1' : '0'; // compact if-else
        num >>= 1;
    }
    printf("%s", binary);
}
