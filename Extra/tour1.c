// tour1.c, C programming light introduction in Lecture 4 in TDT4258 - 2024 edition
// Comments should be sent to Lasse Natvig.
// version 1.0 --- in lecture 2024-09-17
// version 1.1.2 --- 2024-09-23, added command line arguments, bit-masking ops and short circuiting && (and) and || (or) 
//
// Language documentation: https://en.cppreference.com/w/c

// Documentation of headers: https://en.cppreference.com/w/c/header
#include <stdio.h> // for printf 
#include <string.h> // for strcpy 
#include <stdint.h> // for uint8_t, uint16_t, uint32_t.
#include <stdbool.h> // for bool, true, false in earlier standards than C23. In C23 it is part of the standard
  // See also https://en.cppreference.com/w/c/types/boolean


void dataTypes() {
    // Data types
    int a = 100; // The size of these integer types can be different from system to system
    short n1 = 200;
    long n2 = 300;
    float b = 10.5;
    double c = 10.5;
    char d = 'A';
    printf("a: %d, b: %f, c: %lf, d: %c\n", a, b, c, d);
    printf("sizeof(n1): %zu, sizeof(a): %zu, sizeof(n2): %zu\n", sizeof(n1), sizeof(a), sizeof(n2));

    uint8_t e = 0b00001111; // initialized with a binary constant
    uint16_t f = 0b0000111100001111; // these are useful when you need to guarantee the size of a number
    uint32_t g = 0x12345678; // hexadecimal notation

    bool logic = true; // bool, true and false OK from C23, but requires stdbool.h in earlier standards
    if (logic) {
        printf("logic is true\n");
        logic = false;
    }

}    

void arrays() {
    // Arrays
    int arr[5] = {1, 2, 3, 4, 5}; // initialization of all members
    for (int i = 0; i < 5; i++) { // a classic C for loop
        printf("arr[%d]: %d ", i, arr[i]); // %d is format specifier, there are many of them
                                        // https://en.cppreference.com/w/c/io/fprintf                                      
    }
    printf("\n");
    // 2D array of chars
    char arr2D[3][4] = {
        {'A', 'B', 'C', 'D'},
        {'E', 'F', 'G', 'H'},
        {'I', 'J', 'K', 'L'}
    };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            printf("arr2D[%d][%d]: %c\n", i, j, arr2D[i][j]);
        }
    }
    char multiDimArray[3][3][3][3][3][5][4];    // Many dimensions is possible
    printf("\n");
}

void structs() {
    // Structs
    struct Person {
        char name[50];
        int age;
        float salary;
    };
    struct Person p1;
    struct Person* personPtr = &p1; // Pointer to a struct

    strcpy(p1.name, "Dennis Ritchie"); // strcpy is a function to copy strings
    p1.age = 30; // Accessing struct member via dot-notation
    p1.salary = 50000.0; 
    personPtr->age = 31; // Accessing struct members using a pointer
    printf("Person: %s, %d, %f\n", p1.name, p1.age, p1.salary);
}

bool returnFalse() { // These two functions are used to illustrate short-circuit evaluation of composite boolean expressions
    printf("returning false\n");
    return false;
}
bool returnTrue() {
    printf("returning true\n");
    return true;
}

void controlFlow() {
    // Control flow
    int a = 10;
    if (a > 5) {
        printf("a is greater than 5\n");
    } else {
        printf("a is less than or equal to 5\n");
    }
    int b = 2;
    switch (b) {  // called case statement in some languages
        case 1:
            printf("b is 1\n");
            break; // if you forget this one, control flow will "fall through" and continue on case 2: 
        case 2:
            printf("b is 2\n");
            break; // leaves switch statement
        default:
            printf("b is neither 1 nor 2\n");
    }
    int c = 0;
    while (c++ < 5) {  //postincrement operator
        if (c == 3) {
            continue; // continues the loop, skippng rest
        }
        if (c == 4) {
            break; // breaks out of the loop
        }
        printf("c: %d\n", c);
    }
    bool logic1 = true;
    bool logic2 = false;
    if (logic1 || returnFalse()) { // short-circuit evaluation or
        printf("branch1, returnFalse() was NOT called\n");
    }
    if (logic2 && returnTrue()) { // short-circuit evaluation and
        printf("branch2\n");
    } else {
        printf("branch3, returnTrue() was NOT called\n");
    }
}

void printBinary(unsigned char num) {
    char binary[9]; // 8 bits + null terminator
    binary[8] = '\0'; // null terminator
    for (int i = 7; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0'; // compact if-else
        num >>= 1;
    }
    printf("%s", binary);
}
void bitMasking() {
    // Bit masking
    unsigned char a = 0b00001111;
    unsigned char b = 0b11110000;
    unsigned char c = a & b; // bitwise AND
    unsigned char d = a | b; // bitwise OR
    unsigned char e = a ^ b; // bitwise XOR
    unsigned char f = ~a; // bitwise NOT
    printf("c: "); printBinary(c); printf("\n");
    printf("d: "); printBinary(d); printf("\n");
    printf("e: "); printBinary(e); printf("\n");
    printf("f: "); printBinary(f); printf("\n");
}

void shiftOperators() {
    // Shift operators
    unsigned char a = 0b00000010;
    unsigned char b = a << 4; // bitwise shift left 4 positions
    unsigned char c = b >> 1; // shift right 1 position
    // printf("b: %d\n", b); // NOTE we don't need these printfs --- look in the debugger
}

void fileHandling() {
    // File handling
    FILE *fp;
    fp = fopen("file.txt", "w");
    if (fp == NULL) {
        printf("Failed to open file\n");
        return;
    }
    fprintf(fp, "Hello, file handling in C\n");
    fclose(fp);
    printf("File written successfully\n");
} // you can watch it now in VS code, it's in the same folder as the source code
    
void printArguments(int argc, char* argv[]) {
    printf("Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }
}
int main(int argc, char* argv[]) { // main function, the entry point of the program
    printArguments(argc, argv);
    dataTypes();
    arrays();
    structs();
    controlFlow();
    bitMasking();
    shiftOperators();
    fileHandling();
    return 0;
}