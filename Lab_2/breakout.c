/***************************************************************************************************
 * DON'T REMOVE THE VARIABLES BELOW THIS COMMENT                                                   *
 **************************************************************************************************/
unsigned long long __attribute__((used)) VGAaddress = 0xc8000000; // Memory storing pixels
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;

unsigned char n_cols = 10; // <- This variable might change depending on the size of the game. Supported value range: [1,18]

char *won = "You Won";       // DON'T TOUCH THIS - keep the string as is
char *lost = "You Lost";     // DON'T TOUCH THIS - keep the string as is
unsigned short height = 240; // DON'T TOUCH THIS - keep the value as is
unsigned short width = 320;  // DON'T TOUCH THIS - keep the value as is
char font8x8[128][8];        // DON'T TOUCH THIS - this is a forward declaration
/**************************************************************************************************/

/***
 * TODO: Define your variables below this comment
 */


typedef struct _ball
{
    // ballen er en 7x7 piksel firkant
    // dette er koordinatene til den midterste pikselen til ballen
    // bruker disse til å finne posisjonene til venstre, høyre, øvre og nedre piksel når vi vil sjekke om ballen har truffet noe
    unsigned int middle_pos_x;
    unsigned int middle_pos_y;
    unsigned int degrees;
    unsigned int middle_pos_x_old;
    unsigned int middle_pos_y_old;
} Ball;

typedef struct _bar
{
    // baren er en 7x45 piksel firkant
    // dette er koordinatene til den midterste pikselen til baren
    unsigned int middle_pos_x;
    unsigned int middle_pos_y;
} Bar;

/***
 * You might use and modify the struct/enum definitions below this comment
 */
typedef struct _block
{
    // en blokk er en 15x15 piksel firkant
    unsigned char destroyed;
    unsigned char deleted;
    // dette er koordinatene til den midterste pikselen til en blokk
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
} Block;

typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;

GameState currentState = Stopped;
Ball ball = {11, 120, 90, 0, 0};
Bar bar = {4, 112};
Block blocks[288]; // lager en liste med plass til maks antall blokker vi aksepterer, nemlig 18 * 240/15 = 288

/***
 * Here follow the C declarations for our assembly functions
 */

// TODO: Add a C declaration for the ClearScreen assembly procedure
// DONE
void ClearScreen();
void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
void DrawBlock(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color);
void DrawBar(unsigned int y);
int ReadUart();
void WriteUart(char c);

/***
 * Now follow the assembly implementations
 */

// TODO: Add ClearScreen implementation in assembly here
// DONE. CAN MAYBE ALSO REMOVE PUSHING/POPPING OF R4 AD R5 SINCE NOT USED
asm("ClearScreen: \n\t"
    "    PUSH {LR} \n\t"
    "    PUSH {R4, R5} \n\t"
    "    MOV R0, #0\n\t" // flytter start x-koordinat inn i R0
    "    MOV R1, #0\n\t" // flytter start y-koordinat inn i R1
    "    MOV R2, #320\n\t" // flytter VGA bredden inn i R2
    "    MOV R3, #240\n\t" // flytter VGA høyden inn i R3
    "    LDR R4, =0x0000ffff\n\t" // laster hvit inn i R4
    "    PUSH {R4} \n\t" 
    "    BL DrawBlock\n\t" // kaller DrawBlock funksjonen
    "    POP {R4} \n\t" 
    "    POP {R4,R5}\n\t"
    "    POP {LR} \n\t"
    "    BX LR");

// assumes R0 = x-coord, R1 = y-coord, R2 = colorvalue
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3,R1] \n\t"
    "BX LR");

// TODO: Implement the DrawBlock function in assembly. You need to accept 5 parameters, as outlined in the c declaration above (unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color)
// ALMOST DONE. NEED TO FIX INPUT PARAMETERS
asm("DrawBlock: \n\t"
    // TODO: Here goes your implementation
    "PUSH {LR} \n\t"          // Save the return address
    "PUSH {R4-R10} \n\t"       // Save registers that will be used

    // antar at vi får x fra R0, y fra R1, width fra R2, height fra R3 og color fra stack
    "MOV R4, R0 \n\t"         // R4 = x
    "MOV R5, R1 \n\t"         // R5 = y
    "MOV R6, R2 \n\t"         // R6 = width
    "MOV R7, R3 \n\t"         // R7 = height
    "LDR R8, [SP, #32] \n\t"    // R8 = color (5th argument is on the stack at SP+40)

    "mov R9, #0\n\t"
    "mov R10, #0\n\t"
    "loop:\n\t"
    "MOV R0, R4\n\t"      // flytter x-verdien fra R4 til R0 for kall til SetPixel
    "MOV R1, R5\n\t"      // flytter y-verdien fra R5 til R1 for kall til SetPixel
    "MOV R2, R8 \n\t"     // flytter fargen fra R8 til R2 for kall til SetPixel
    "BL SetPixel\n\t"     // kaller SetPixel funksjonen
    "add R4, R4, #1\n\t"  // inkrementerer x med 1
    "add R9, R9, #1\n\t"  // inkrementerer loop counter som holder styr på hvor i raden vi er
    "cmp R9, R6\n\t"     // sjekker om counteren er blitt lik 10, altså bredden på blokken
    "blt loop\n\t"        // hvis mindre enn 10, gjentar vi prosessen med oppdatert x-koordinat
    "sub R4, R4, R6\n\t" // hvis lik 10, resetter vi x-koordinaten
    "mov R9, #0\n\t"      // resetter loop counteren for x-koordinaten
    "add R5, R5, #1\n\t"  // inkrementerer y, altså hopper til neste rad
    "add R10, R10, #1\n\t"  // Inkrementerer y sin counter
    "cmp R10, R7\n\t"     // sammenligner y sin counter med 10
    "blt loop\n\t"        // hvis mindre enn 10, fortsetter vi å fylle piksler i denne raden

    "POP {R4-R10} \n\t"  // gjenoppretter registerene fra stacken
    "POP {LR} \n\t"     // gjenoppretter return adressen fra stacken
    
    "BX LR"); // returnerer til funksjonen vi ble kalt fra

// TODO: Impelement the DrawBar function in assembly. You need to accept the parameter as outlined in the c declaration above (unsigned int y)
// DONE. CAN MAYBE ALSO MAKE THE UPPER, MIDDLE AND LOWER PARTS DIFFERENT COLORS
// antar at unsigned int y kommer gjennom R0-registeret
asm("DrawBar: \n\t"
    "    PUSH {LR} \n\t"
    "    PUSH {R4, R5} \n\t"
    
    "    MOV R1, R0\n\t" // flytter start y-koordinat fra input-en i R0 inn i R1
    "    MOV R0, #0\n\t" // flytter start x-koordinat inn i R0
    "    MOV R2, #7\n\t" // flytter bar bredden (x-retning) inn i R2
    "    MOV R3, #45\n\t" // flytter bar høyden (y-retning) inn i R3
    "    LDR R4, =0x000000ff\n\t" // lagrer blå i R4
    "    PUSH {R4} \n\t"         // Pusher fargen på stacken
    "    BL DrawBlock\n\t" // kaller DrawBlock funksjonen

    "    POP {R4} \n\t"          // Pop the color off the stack
    "    POP {R4,R5}\n\t"
    "    POP {LR} \n\t"
    "    BX LR");

asm("ReadUart:\n\t"
    "LDR R1, =0xFF201000 \n\t"
    "LDR R0, [R1]\n\t"
    "BX LR");

// TODO: Add the WriteUart assembly procedure here that respects the WriteUart C declaration on line 46
// DONE
asm("WriteUart:\n\t"
    "LDR R1, =0xFF201000 \n\t"
    "STR R0, [R1]\n\t"
    "BX LR");


// TODO: Implement the C functions below
void draw_ball()
{
    // vil finne koordinat til øvre venstre piksel, og lage en block
    DrawBlock(ball.middle_pos_x_old - 3, ball.middle_pos_y_old - 3, 7, 7, white);
    DrawBlock(ball.middle_pos_x - 3, ball.middle_pos_y - 3, 7, 7, green);

}

void draw_playing_field()
{
    for (int i = 0; i < 16 * n_cols; i++) {
        if (blocks[i].destroyed == 1 && blocks[i].deleted == 0) {
            DrawBlock(blocks[i].pos_x - 7, blocks[i].pos_y - 7, 15, 15, white);
            blocks[i].deleted = 1;
        }
    }
}

void wait()
{
    volatile int j = 0;
    while (j < 10000) {
        j++;
    }
}

void update_game_state()
{
    ball.middle_pos_x_old = ball.middle_pos_x;
    ball.middle_pos_y_old = ball.middle_pos_y;

    if (currentState != Running)
    {
        return;
    }

    // TODO: Check: game won? game lost?
    // Må sjekke om ballens høyre x-koordinat er 320. Hvis sant, oppdater til won
    if (ball.middle_pos_x + 3 >= 320)
    {
        currentState = Won;
        return;
    }
    // Må sjekke om ballens venstre x-koordinat er mindre enn 7. Hvis sant, oppdater til lost
    if (ball.middle_pos_x - 3 < 7)
    {
        currentState = Lost;
        return;
    }
    // Fjerner ballens gamle posisjon fra ui
    //DrawBlock(ball.middle_pos_x - 3, ball.middle_pos_y - 3, 7, 7, white);

    // TODO: Update balls position and direction
    // lar ballen bevege seg med 1 piksel per update

    if (ball.degrees == 0) { // ball is going straight up
        ball.middle_pos_y -= 1;
    } else if (ball.degrees == 180) { // ball is going straight down
        ball.middle_pos_y += 1;
    } else if (ball.degrees == 90) { // ball is going straight right
        ball.middle_pos_x += 1;
    } else if (ball.degrees == 270) { // ball is going straight left
        ball.middle_pos_x -= 1;
    } else if (ball.degrees == 45) { // ball is going up-right
        ball.middle_pos_x += 1;
        ball.middle_pos_y -= 1;
    } else if (ball.degrees == 135) { // ball is going down-right
        ball.middle_pos_x += 1;
        ball.middle_pos_y += 1;
    } else if (ball.degrees == 315) { // ball is going up-left
        ball.middle_pos_x -= 1;
        ball.middle_pos_y -= 1;
    } else if (ball.degrees == 225) { // ball is going down-left
        ball.middle_pos_x -= 1;
        ball.middle_pos_y += 1;
    }

    // TODO: Hit Check with Blocks
    // HINT: try to only do this check when we potentially have a hit, as it is relatively expensive and can slow down game play a lot

    // for å ikke sjekke for ofte, sjekker vi kun når ballen er innenfor området der det har vært blokker. Trekker fra 5 pga. bruker middle_pos_x, som er 3 unna ytterkanten
    if (ball.middle_pos_x > 320 - n_cols * 15 - 5)
    {
        for (int i = 0; i < 16 * n_cols; i++) {

            if (blocks[i].destroyed == 0) {

                if (ball.middle_pos_x + 3 > blocks[i].pos_x - 7 && ball.middle_pos_x - 3 < blocks[i].pos_x + 7) {
                    if (ball.middle_pos_y + 3 == blocks[i].pos_y - 7) { // ball bunn treffer blokk topp
                        if (ball.degrees == 45){
                            ball.degrees = 135;
                        } else if (ball.degrees == 315) {
                            ball.degrees = 225;
                        }
                        blocks[i].destroyed = 1;
                    } else if (ball.middle_pos_y - 3 == blocks[i].pos_y + 7) { // ball topp treffer blokk bunn
                        if (ball.degrees == 135){
                            ball.degrees = 45;
                        } else if (ball.degrees == 225) {
                            ball.degrees = 315;
                        }
                        blocks[i].destroyed = 1;
                    }

                } else if (ball.middle_pos_y + 3 > blocks[i].pos_y - 7 && ball.middle_pos_y - 3 < blocks[i].pos_y + 7) {
                    if (ball.middle_pos_x + 3 == blocks[i].pos_x - 7) { // ball høyre treffer blokk venstre
                        if (ball.degrees == 45){
                            ball.degrees = 315;
                        } else if (ball.degrees == 90) {
                            ball.degrees = 270;
                        } else if (ball.degrees == 135) {
                            ball.degrees = 225;
                        }
                        blocks[i].destroyed = 1;
                    } else if (ball.middle_pos_x - 3 == blocks[i].pos_x + 7) { // ball venstre treffer blokk høyre
                        if (ball.degrees == 315){
                            ball.degrees = 45;
                        } else if (ball.degrees == 225) {
                            ball.degrees = 135;
                        }
                        blocks[i].destroyed = 1;
                    }

                }

            }
        }
    }

    // sjekker om ballen har truffet topp eller bunn. I så fall, endrer vi vinkel
    if (ball.middle_pos_y - 4 == 0) { // treffer topp
        if (ball.degrees == 45) {
            ball.degrees = 135;
        } else if (ball.degrees == 315) {
            ball.degrees = 225;
        } else if (ball.degrees == 0) {
            ball.degrees = 180;
        }
    } else if (ball.middle_pos_y + 4 == 240) { // treffer bunn
        if (ball.degrees == 135) {
            ball.degrees = 45;
        } else if (ball.degrees == 225) {
            ball.degrees = 315;
        } else if (ball.degrees == 180) {
            ball.degrees = 0;
        }
    }

    // Sjekker så om ballen har truffet baren. I så fall får vi den til å snu retning igjen
    if (ball.middle_pos_x - 3 == bar.middle_pos_x + 3) {
        if (ball.middle_pos_y <= bar.middle_pos_y + 7 && ball.middle_pos_y >= bar.middle_pos_y - 7) {
            ball.degrees = 90;
        } else if (ball.middle_pos_y <= bar.middle_pos_y + 23 && ball.middle_pos_y >= bar.middle_pos_y + 8) {
            ball.degrees = 135;
        } else if (ball.middle_pos_y <= bar.middle_pos_y - 8 && ball.middle_pos_y >= bar.middle_pos_y - 23) {
            ball.degrees = 45;
        } else if (ball.middle_pos_y - 3 <= bar.middle_pos_y + 23 && ball.middle_pos_y + 3 > bar.middle_pos_y + 23) {
            ball.degrees = 135;
        } else if (ball.middle_pos_y + 3 >= bar.middle_pos_y - 23 && ball.middle_pos_y - 3 < bar.middle_pos_y - 23) {
            ball.degrees = 45;
        }
    }

}

void update_bar_state()
{
    int move_step = 15;
    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid - abort reading
            return;
        }
        unsigned char char_received = out & 0xFF;
        if (char_received == 0x0a) {
            currentState = Exit;

        } else if (char_received == 0x77) { // 'w' key
            DrawBlock(bar.middle_pos_x - 4, bar.middle_pos_y - 23, 7, 45, white); // fjerner gammel bar fra ui

            if (bar.middle_pos_y - 23 - move_step >= 0) {
                bar.middle_pos_y -= move_step;  // Implement the logic to move the bar upwards
            } else {
                bar.middle_pos_y = 23;
            }
            DrawBar(bar.middle_pos_y - 23); // oppdaterer baren i ui
        } else if (char_received == 0x73) { // 's' key
            DrawBlock(bar.middle_pos_x - 4, bar.middle_pos_y - 23, 7, 45, white); // fjerner gammel bar fra ui

            if (bar.middle_pos_y + 23 + move_step <= 240) {
                bar.middle_pos_y += move_step;
            } else {
                bar.middle_pos_y = 217;
            }
            DrawBar(bar.middle_pos_y - 23); // oppdaterer baren i ui
        }
        remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);
}

void write(char *str)
{
    // TODO: Use WriteUart to write the string to JTAG UART
    // DONE
    while (*str) {
        WriteUart(*str);
        str++;
    }
}

void initialize_blocks() {
    int blockIndex = 0;
    unsigned int colors[] = {red, green, blue, black}; // Example color palette
    unsigned int color_index = 0;

    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < n_cols; col++) {
            blocks[blockIndex].destroyed = 0;       // All blocks start intact
            blocks[blockIndex].deleted = 0;         // No blocks are deleted at the start
            blocks[blockIndex].pos_x = 320 - col * 15 + 7;  // X position of middle pixel based on column.
            blocks[blockIndex].pos_y = row * 15 + 7; // Y position of middle pixel based on row

            // velger neste farge via color_index. Passer på å velge annen enn nabo-blokker ved å inkrementere color_index og prøve på nytt hvis fargene er like
            unsigned int chosen_color = colors[color_index % 4];
            if (row > 0 && blocks[blockIndex - n_cols].color == chosen_color) {
                color_index++;
                chosen_color = colors[color_index % 4];
            }
            if (col > 0 && blocks[blockIndex - 1].color == chosen_color) {
                color_index++;
                chosen_color = colors[color_index % 4];
            }
            blocks[blockIndex].color = chosen_color;

            DrawBlock(320 - col * 15, row * 15, 15 - 1, 15 - 1, chosen_color); // tegner selve blokken

            blockIndex++;
            color_index++;
        }
    }
}


void play()
{
    ClearScreen();
    initialize_blocks();
    currentState = Running;
    // HINT: This is the main game loop
    while (1)
    {
        update_game_state();
        update_bar_state();
        if (currentState != Running)
        {
            break;
        }
        //ClearScreen();
        wait();
        draw_playing_field();
        draw_ball();
        DrawBar(bar.middle_pos_y - 23); // Finner y-koordinaten til øverste piksel i baren
    }
    if (currentState == Won)
    {
        write(won);
    }
    else if (currentState == Lost)
    {
        write(lost);
    }
    else if (currentState == Exit)
    {
        return;
    }
    currentState = Stopped;
}

void reset()
{
    // Hint: This is draining the UART buffer
    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid - abort reading
            return;
        }
        remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);

    // TODO: You might want to reset other state in here
    // resetter koordinatene
    ball.middle_pos_x = 11;
    ball.middle_pos_y = 120;
    bar.middle_pos_y = 120;
    
    // Legger til alle blokkene igjen
    for (int i = 0; i < 16 * n_cols; i++) {
        blocks[i].destroyed = 0;
        blocks[i].deleted = 0;
    }
}

void wait_for_start()
{
    return;
    // TODO: Implement waiting behaviour until the user presses either w/s
    // unsigned long long uart_data = 0;
    // unsigned char char_received = 0;

    // write("Press 'w' to start moving up, or 's' to start moving down...\n");

    // while (1) {
    //     // Continuously read from UART buffer
    //     uart_data = ReadUart();

    //     // Check if the UART buffer is ready (MSB 0x8000 flag set)
    //     if (!(uart_data & 0x8000)) {
    //         continue; // If not ready, continue waiting
    //     }

    //     // Extract the actual character received (lower 8 bits of the data)
    //     char_received = uart_data & 0xFF;

    //     // Check if the character is 'w' or 's'
    //     if (char_received == 0x77 || char_received == 0x73) {
    //         // If 'w' or 's' is received, break the loop and start the game
    //         write("Game starting...\n");
    //         return;
    //     }
    // }
}

int main(int argc, char *argv[])
{
    ClearScreen();
    if (n_cols < 1 || n_cols > 18) {
        char *illegal_n_cols = "This is not a playable configuration. n_cols must be in range [1, 18].";
        write(illegal_n_cols);
        return 0;
    }

    // HINT: This loop allows the user to restart the game after loosing/winning the previous game
    while (1)
    {
        wait_for_start();
        play();
        reset();
        if (currentState == Exit)
        {
            break;
        }
        return 0;
    }
    return 0;
}

// THIS IS FOR THE OPTIONAL TASKS ONLY

// HINT: How to access the correct bitmask
// sample: to get character a's bitmask, use
// font8x8['a']
char font8x8[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // U+007F
};