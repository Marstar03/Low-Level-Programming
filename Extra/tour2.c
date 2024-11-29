// TDT4258 autumn 2024, code to test some slides in Lecture 5, Lasse Natvig.
//
#include <stdio.h> // Needed for printf
#include<stdlib.h> // Needed for malloc
#include<string.h> // Needed for memcpy
void slide17() {
    int a = 524353; 
    short b;
    b = a;
    printf("slide 17: %d\n", b);
}

void slide18() {
    int a = 32833; 
    short b;
    b = a;
    printf("slide 18: %d\n", b);
}

void slide20() {
	int a;
	a = 8;
	short b;
	b = *(short*)&a;
    printf("slide 20: %d\n", b);
}

void slide21(){
	short a = 8; 
	int b; // L.E. = little endian
    b = *(int*)&a;
    printf("slide 21: %d\n", b);
    // masking to remove the unknown xxxx ... bits in the slide
	printf("masked on L.E. computer  %d\n", b & 0x0000ffff);  // slide 21    
}

void slide23() {
    int dummy1[ ] = {1, 2, 3, 4, 5};
    int arr[6];  // indexed by 0..5 
    arr[0] = 4;
    arr[5] = 40;
    arr[6] = 20;
    arr[10] = 30;
    arr[-3] = 10;
}

typedef struct point {
    int x;
    int y;
	int dummy1;
} point;

void slide29_30(){
    point p1;
    p1.x = 10;
    p1.y = 20;
    ((point* )&(p1.y))->x = 30;
    ((point* )&(p1.y))->y = 40;
    printf("slides 29-30: %d\n", p1.dummy1);
}

void swap(void* a, void* b, int size){
    char* buffer;
    if (( buffer = malloc(size)) == NULL) {
        printf("error: could not allocate memory\n");
    } 
    memcpy(buffer, a, size);
    memcpy(a, b, size);
    memcpy(b, buffer, size);
}

void slide35() {
    int x = 10;
    int y = 20;
    swap(&x, &y, sizeof(int));
    printf("slide 35: x, y: %d, %d\n", x, y);
    float f = 3.14; // more testing
    float g = 6.28;
    swap(&f, &g, sizeof(float));  
    printf("slide 35+: f, g: %f, %f", f, g);  
}


int main(int argc, char *argv[]) {
	short a; int b;
	printf("size of short, int: %zu, %zu\n", sizeof(a), sizeof(b));
    slide17();  
    slide18();  
	slide20();  
	slide21(); 
    //slide23();  
    slide29_30();
    slide35();
    return 0;
}