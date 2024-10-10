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

// Struct for keeping track of the coordinates and angle of the ball
typedef struct _ball
{
    // The ball is a 7x7 oixel square.
    // These are the coordinates of the middle pixel of the ball.
    // Using these for finding the positions of the left, right, upper and lower pixel
    // when we check if the ball has hit something
    int middle_pos_x;
    int middle_pos_y;
    unsigned int degrees;
    // Also keeping track of the previous coordinates of the ball so that we can remove the old ball
    // and draw the new ball right after eachother for a smoother experience
    int middle_pos_x_old;
    int middle_pos_y_old;
} Ball;

// Struct for keeping track of the position of the bar
typedef struct _bar
{
    // The bar is a 7x45 pixel rectangle.
    // These are the coordinates of the middle pixel of the bar.
    int middle_pos_x;
    int middle_pos_y;
} Bar;

/***
 * You might use and modify the struct/enum definitions below this comment
 */
typedef struct _block
{
    // A block is a 15x15 pixel square
    unsigned char destroyed;
    unsigned char deleted;
    // These are the coordinates of the middle pixel of a block
    unsigned int pos_x;
    unsigned int pos_y;
    unsigned int color;
    // These are the row and column number of a block. Used when checking if a block has an adjacent block
    unsigned int row;
    unsigned int col;
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
Ball ball = {11, 120, 90, 11, 120};
Bar bar = {4, 112};
// Creating a list with space for the max number of blocks we accept, which is 18 * 240/15 = 288
Block blocks[288];

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
// DONE
asm("ClearScreen: \n\t"
    "PUSH {LR} \n\t" // Pushing the link register to stack to be able to return from caller
    "PUSH {R4} \n\t" // Pushing old R4 register value to stack since it will get overwritten
    "MOV R0, #0 \n\t" // Moving the start x-coordinate into R0
    "MOV R1, #0 \n\t" // Moving the start y-coordinate into R1
    "MOV R2, #320 \n\t" // Moving the VGA width into R2
    "MOV R3, #240 \n\t" // Moving the VGA height into R3
    "LDR R4, =0x0000ffff \n\t" // Loading the color white into R4
    "PUSH {R4} \n\t" // Need to pass 5th parameter (color) through stack, so pushing to stack
    "BL DrawBlock \n\t" // Calling the DrawBlock assembly function by branching and linking
    "POP {R4} \n\t" // popping the R4 value from the stack again after it has been used
    "POP {R4} \n\t" // Popping the old R4 value from stack
    "POP {LR} \n\t" // Popping the old LR value from stack
    "BX LR"); // Returning from function

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
// DONE
asm("DrawBlock: \n\t"
    // TODO: Here goes your implementation
    "PUSH {LR} \n\t" // Saving the return address on the stack since it will be overwritten when calling SetPixel
    "PUSH {R4-R10} \n\t" // Save registers that will be used

    // Assuming that we get the x value from R0, y from R1, width from R2, height from R3 and color from the stack
    "MOV R4, R0 \n\t" // Moving x into R4
    "MOV R5, R1 \n\t" // Moving y into R5
    "MOV R6, R2 \n\t" // Moving the width into R6
    "MOV R7, R3 \n\t" // Moving the height into R7
    "LDR R8, [SP, #32] \n\t" // Loading the color from the stack into R8. Using SP + 32 since LR and R4-R10 (4 bytes each) are stored on stack, so need to access stack with (1 + 7)*4 = 32 bytes offset

    "MOV R9, #0 \n\t" // Initializing the R9 register as 0 used for counting current column
    "MOV R10, #0 \n\t" // Initializing the R10 register as 0 used for counting current row
    "block_loop: \n\t"
    "MOV R0, R4 \n\t" // Moving the x value from R4 into R0 for call to SetPixel
    "MOV R1, R5 \n\t" // Moving the y value from R5 into R1 for call to SetPixel
    "MOV R2, R8 \n\t" // Moving the color from R8 into R2 for call to Setpixel
    "BL SetPixel \n\t" // Calling the SetPixel function
    "ADD R4, R4, #1 \n\t" // Incrementng x with 1
    "ADD R9, R9, #1 \n\t" // Incrementing the loop counter keeping track of where in the row we are
    "CMP R9, R6 \n\t" // Checking if the loop counter is equal to the width of the block
    "BLT block_loop \n\t" // If less than the width, then repeat the process with updated x
    "SUB R4, R4, R6 \n\t" // Resetting the x coordinate if equal to the width
    "MOV R9, #0 \n\t" // Resetting the loop counter for the x coordinate
    "ADD R5, R5, #1 \n\t" // incrementing y, jumping to the next row
    "ADD R10, R10, #1 \n\t"  // Incrementing the y counter
    "CMP R10, R7 \n\t" // Comparing the y counter to the height of the block
    "BLT block_loop \n\t" // If less than the width, then we keep filling pixels in this row

    "POP {R4-R10} \n\t" // Restoring the registers from the stack
    "POP {LR} \n\t" // Restoring the return address from the stack
    
    "BX LR"); // Returning to the function we got called from

// TODO: Impelement the DrawBar function in assembly. You need to accept the parameter as outlined in the c declaration above (unsigned int y)
// DONE. CAN MAYBE ALSO MAKE THE UPPER, MIDDLE AND LOWER PARTS DIFFERENT COLORS
asm("DrawBar: \n\t"
    "PUSH {LR} \n\t"
    "PUSH {R4, R5, R6, R7} \n\t"
    
    // Assuming that the y coordinate of the bar comes from the R0 register

    "MOV R5, R0 \n\t" // Moving the start y coordinate from R0 to R5

    // Setting up the loop for drawing the 3 parts of the bar
    "MOV R6, #3 \n\t" // Loop counter for 3 parts
    "MOV R7, #0 \n\t" // Part index

    // Drawing the top part of the bar
    "bar_loop: \n\t"
    "MOV R1, R5 \n\t" // Moving the start y coordinate from R5 to R1
    "MOV R0, #0 \n\t" // Moving the start x coordinate into R0
    "MOV R2, #7 \n\t" // Moving the bar width into R2
    "MOV R3, #15 \n\t" // Moving the height into R3

    // Setting the color based on the part index
    "CMP R7, #1 \n\t" // Comparing part index with 1
    "BEQ middle_part \n\t" // If equal, jumping to middle_part
    "LDR R4, =0x000000ff \n\t" // Storing the value of the color blue in R4
    "B draw_part \n\t" // Jumping to draw_part

    "middle_part: \n\t"
    "LDR R4, =0x0000f0f0 \n\t" // Storing the value of the color red in R4

    "draw_part: \n\t"
    "PUSH {R4} \n\t" // Pushing the color onto the stack
    "BL DrawBlock \n\t" // Calling the DrawBlock function
    "POP {R4} \n\t" // Popping the color off the stack

    "ADD R5, R5, #15 \n\t" // Increasing the y coordinate to point to upper left pixel in next part
    "ADD R7, R7, #1 \n\t" // Incrementing the part index
    "CMP R7, R6 \n\t" // Comparing the part index to the loop counter
    "BLT bar_loop \n\t" // If less than the loop counter, repeat the loop

    "POP {R4, R5, R6, R7} \n\t" // Popping the old register values from stack
    "POP {LR} \n\t" // Popping the old LR value from stack
    "BX LR"); // Returning

asm("ReadUart: \n\t"
    "LDR R1, =0xFF201000 \n\t"
    "LDR R0, [R1] \n\t"
    "BX LR");

// TODO: Add the WriteUart assembly procedure here that respects the WriteUart C declaration on line 46
// DONE
asm("WriteUart: \n\t"
    // Assuming that the char being written to the UART is passed through R0
    "LDR R1, =0xFF201000 \n\t" // Loading the address of the UART into R1
    "STR R0, [R1] \n\t" // Storing the value in R0 at the address of the UART
    "BX LR");


// TODO: Implement the C functions below
// DONE
void draw_ball()
{
    DrawBlock(ball.middle_pos_x_old - 3, ball.middle_pos_y_old - 3, 7, 7, white); // First removing the old ball from the VGA by coloring it white
    DrawBlock(ball.middle_pos_x - 3, ball.middle_pos_y - 3, 7, 7, green); // Then drawing a new ball at the updated coordinates

}

// Instead of drawing the whole playing field again, i use this function to remove the blocks
// that have been deleted/hit, which is more efficient
void draw_playing_field()
{
    // Iterating over each block
    for (int i = 0; i < 16 * n_cols; i++) {
        // Checking if the block is destroyed and not deleted
        if (blocks[i].destroyed == 1 && blocks[i].deleted == 0) {
            // If so, we remove the block from the UI and set it as deleted
            DrawBlock(blocks[i].pos_x - 8, blocks[i].pos_y - 7, 15, 15, white);
            blocks[i].deleted = 1;
        }
    }
}

// Function that gets called for each iteration in the play function, for the ball not to move too fast
void wait()
{
    // Slowing down the process by counting to 10000
    volatile int j = 0;
    while (j < 10000) {
        j++;
    }
}

void update_game_state()
{
    // First setting the old ball coordinates before we update to the new ones
    ball.middle_pos_x_old = ball.middle_pos_x;
    ball.middle_pos_y_old = ball.middle_pos_y;

    // Exiting the function if not in running state
    if (currentState != Running)
    {
        return;
    }

    // TODO: Check: game won? game lost?
    // DONE

    // Checking if the right ball x coordinate is 320. If so, update the state to won and return
    if (ball.middle_pos_x + 3 >= width)
    {
        currentState = Won;
        return;
    }

    // Checking if the left ball x coordinate is less than 7. If so, update the state to lost and return
    if (ball.middle_pos_x - 3 < 7)
    {
        currentState = Lost;
        return;
    }

    // TODO: Update balls position and direction
    // DONE

    // Letting the ball move with 1 pixel per update. Comparing the ball degrees to check direction
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

    // In order to not check too often, we check only when the ball is within the area where there is/has been blocks. 
    // Subtracting 3 since we use the middle x coordinate, which is 3 from the border
    if (ball.middle_pos_x > width - n_cols * 15 - 3)
    {
        // Iterating over the blocks
        for (int i = 0; i < 16 * n_cols; i++) {
            // Checking that the block hasnt already been destroyed
            if (blocks[i].destroyed == 0) {

                // First checking if a ball corner is going straight towards a block corner
                if ((ball.middle_pos_x + 3 == blocks[i].pos_x - 8 && ball.middle_pos_y + 3 == blocks[i].pos_y - 8 && ball.degrees == 135)
                || (ball.middle_pos_x - 3 == blocks[i].pos_x + 8 && ball.middle_pos_y + 3 == blocks[i].pos_y - 8 && ball.degrees == 225)
                || (ball.middle_pos_x + 3 == blocks[i].pos_x - 8 && ball.middle_pos_y - 3 == blocks[i].pos_y + 8 && ball.degrees == 45)
                || (ball.middle_pos_x - 3 == blocks[i].pos_x + 8 && ball.middle_pos_y - 3 == blocks[i].pos_y + 8 && ball.degrees == 315)) {

                    // If so, it makes sense that the direction turns 180 degrees
                    ball.degrees = (ball.degrees + 180) % 360;
                    blocks[i].destroyed = 1;

                // Else, checking if the middle x coordinate is within the block x coordinates
                } else if (ball.middle_pos_x >= blocks[i].pos_x - 7 && ball.middle_pos_x <= blocks[i].pos_x + 7) {
                    // Checking if the bottom middle y has hit the top of the block (ball bottom hits block top)
                    if (ball.middle_pos_y + 3 >= blocks[i].pos_y - 8 && ball.middle_pos_y + 3 < blocks[i].pos_y + 7) { 
                        // Reflecting according to physics
                        if (ball.degrees == 135){
                            ball.degrees = 45;
                        } else if (ball.degrees == 225) {
                            ball.degrees = 315;
                        }
                        // Deleting the block
                        blocks[i].destroyed = 1;
                        
                        // If the bottom middle hits the corner pixel of the block, then the neighbour block will also be destroyed
                        if (ball.middle_pos_x == blocks[i].pos_x - 7 && blocks[i].col < n_cols - 1) { // Upper left corner
                            blocks[i + 1].destroyed = 1; // Block to the left gets destroyed
                        } else if (ball.middle_pos_x == blocks[i].pos_x + 7 && blocks[i].col > 0) { // Upper right corner
                            blocks[i - 1].destroyed = 1; // Block to the right gets destroyed
                        }
                    // Checking if the top middle y has hit the bottom of the block (ball top hits block bottom)
                    } else if (ball.middle_pos_y - 3 <= blocks[i].pos_y + 8 && ball.middle_pos_y + 3 > blocks[i].pos_y - 7) {
                        // Reflecting according to physics
                        if (ball.degrees == 45){
                            ball.degrees = 135;
                        } else if (ball.degrees == 315) {
                            ball.degrees = 225;
                        }
                        // Deleting the block
                        blocks[i].destroyed = 1;

                        // If the top middle hits the corner pixel if the block, then the neighbour block will also be destroyed
                        if (ball.middle_pos_x == blocks[i].pos_x - 7 && blocks[i].col < n_cols - 1) { // Lower left corner
                            blocks[i + 1].destroyed = 1; // Block to the left gets destroyed
                        } else if (ball.middle_pos_x == blocks[i].pos_x + 7 && blocks[i].col > 0) { // Lower right corner
                            blocks[i - 1].destroyed = 1; // Block to the right gets destroyed
                        }
                    }

                // Else, checking if the middle y coordinate is within the block y coordinates
                } else if (ball.middle_pos_y >= blocks[i].pos_y - 7 && ball.middle_pos_y <= blocks[i].pos_y + 7) {
                    // Checking if the right middle x has hit the left of the block (ball right hits block left)
                    if (ball.middle_pos_x + 3 >= blocks[i].pos_x - 8 && ball.middle_pos_x - 3 < blocks[i].pos_x + 7) {
                        // Reflecting according to physics
                        if (ball.degrees == 45){
                            ball.degrees = 315;
                        } else if (ball.degrees == 90) {
                            ball.degrees = 270;
                        } else if (ball.degrees == 135) {
                            ball.degrees = 225;
                        }
                        // Deleting the block
                        blocks[i].destroyed = 1;

                        // If the right middle hits the corner pixel if the block, then the neighbour block will also be destroyed
                        if (ball.middle_pos_y == blocks[i].pos_y - 7 && blocks[i].row > 0) { // Upper left corner
                            blocks[i - n_cols].destroyed = 1; // Block above gets destroyed
                        } else if (ball.middle_pos_y == blocks[i].pos_y + 7 && blocks[i].row < 16 - 1) { // Lower left corner
                            blocks[i + n_cols].destroyed = 1; // Block below gets destroyed
                        }

                    // Checking if the left middle x has hit the right of the block (ball left hits block right)
                    } else if (ball.middle_pos_x - 3 <= blocks[i].pos_x + 8 && ball.middle_pos_x + 3 > blocks[i].pos_x - 7) {
                        // Reflecting according to physics
                        if (ball.degrees == 315){
                            ball.degrees = 45;
                        } else if (ball.degrees == 225) {
                            ball.degrees = 135;
                        }
                        // Deleting the block
                        blocks[i].destroyed = 1;

                        // If the left middle hits the corner pixel if the block, then the neighbour block will also be destroyed
                        if (ball.middle_pos_y == blocks[i].pos_y - 7 && blocks[i].row > 0) { // Upper right corner
                            blocks[i - n_cols].destroyed = 1; // Block above gets destroyed
                        } else if (ball.middle_pos_y == blocks[i].pos_y + 7 && blocks[i].row < 16 - 1) { // Lower right corner
                            blocks[i + n_cols].destroyed = 1; // Block below gets destroyed
                        }

                    }

                }

            }
        }
    }

    // Checking if the ball has hit the top or bottom of the VGA. If so, we change the angle
    if (ball.middle_pos_y - 4 == 0) { // Hits the top
        // Reflecting according to physics
        if (ball.degrees == 45) {
            ball.degrees = 135;
        } else if (ball.degrees == 315) {
            ball.degrees = 225;
        } else if (ball.degrees == 0) {
            ball.degrees = 180;
        }
    } else if (ball.middle_pos_y + 4 == height) { // Hits the bottom
        // Reflecting according to physics
        if (ball.degrees == 135) {
            ball.degrees = 45;
        } else if (ball.degrees == 225) {
            ball.degrees = 315;
        } else if (ball.degrees == 180) {
            ball.degrees = 0;
        }
    }

    // Checking if the ball has hit the bar. If so, we make it change direction again,
    // where the updated angle depends on if we hit the top, middle or lower part of the bar
    if (ball.middle_pos_x - 3 == bar.middle_pos_x + 3) {
        // Checking if the ball has hit the middle 15 pixels
        if (ball.middle_pos_y <= bar.middle_pos_y + 7 && ball.middle_pos_y >= bar.middle_pos_y - 7) {
            ball.degrees = 90;

        // Checking if the ball has hit the lower 15 pixels
        } else if (ball.middle_pos_y <= bar.middle_pos_y + 23 && ball.middle_pos_y >= bar.middle_pos_y + 8) {
            ball.degrees = 135;

        // Checking if the ball has hit the upper 15 pixels
        } else if (ball.middle_pos_y <= bar.middle_pos_y - 8 && ball.middle_pos_y >= bar.middle_pos_y - 23) {
            ball.degrees = 45;
        
        // Checking if only the upper part of the ball has hit the bar
        } else if (ball.middle_pos_y - 3 <= bar.middle_pos_y + 23 && ball.middle_pos_y + 3 > bar.middle_pos_y + 23) {
            ball.degrees = 135;

        // Checking if only the lower part of the ball has hit the bar
        } else if (ball.middle_pos_y + 3 >= bar.middle_pos_y - 23 && ball.middle_pos_y - 3 < bar.middle_pos_y - 23) {
            ball.degrees = 45;
        }
    }

}

// Using this function to update state as well as removing old bar in VGA
void update_bar_state()
{
    // The step size to move when pressing w or s
    int move_step = 15;
    int remaining = 0;
    do
    {
        // Reading the next value from the UART buffer
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid, so aborting reading
            return;
        }

        // Retrieving the lower byte of the out value
        unsigned char char_received = out & 0xFF;

        // Checking if the value corresponds to a press on enter button. If so, setting the state to Exit to stop the program
        if (char_received == 0x0a) {
            currentState = Exit;

        // Checking if the value corresponds to a press on w key.
        } else if (char_received == 0x77) {
            DrawBlock(bar.middle_pos_x - 4, bar.middle_pos_y - 23, 7, 45, white);
            
            // Decreasing the bar y coordinate with step size if able to (15 or more pixels from top of bar to top of VGA)
            if (bar.middle_pos_y - 23 - move_step >= 0) {
                bar.middle_pos_y -= move_step;

            // If not enough space, we set it to the top of the screen, meaning that the middle coordinate becomes 23
            } else {
                bar.middle_pos_y = 23;
            }

        // Checking if the value corresponds to a press on s key.
        } else if (char_received == 0x73) {
            DrawBlock(bar.middle_pos_x - 4, bar.middle_pos_y - 23, 7, 45, white); // fjerner gammel bar fra ui

            // Increasing the bar y coordinate with step size if able to (15 or more pixels from bottom of bar to bottom of VGA)
            if (bar.middle_pos_y + 23 + move_step <= height) {
                bar.middle_pos_y += move_step;
            
            // If not enough space, we set it to the bottom of the screen, meaning that the middle coordinate becomes 217
            } else {
                bar.middle_pos_y = 217;
            }
        }
        remaining = (out & 0xFF0000) >> 4;
    } while (remaining > 0);
}

void write(char *str)
{
    // TODO: Use WriteUart to write the string to JTAG UART
    // DONE

    // Writing new character from the string pointed to by the str pointer until there are none left
    while (*str) {
        WriteUart(*str);
        str++;
    }
}

// Function for creating all the blocks that will be used in the game
void initialize_blocks() {
    int blockIndex = 0; // Index for keeping track of which block to initialize
    unsigned int colors[] = {red, green, blue, black}; // Creating an array of the possible block colors
    unsigned int color_index = 0; // Creating an index for keeping track of which color to use on the current block

    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < n_cols; col++) {
            blocks[blockIndex].destroyed = 0; // All blocks start with not being destroyed
            blocks[blockIndex].deleted = 0; // All blocks start with not being deleted
            blocks[blockIndex].pos_x = width - col * 15 - 7; // Setting the x coordinate of middle pixel based on column number. Making sure to fill from the right to the left by subtracting from the width 320
            blocks[blockIndex].pos_y = row * 15 + 7; // Setting the y coordinate of middle pixel based on row number
            blocks[blockIndex].row = row; // Setting the row of the block
            blocks[blockIndex].col = col; // Setting the column of the block

            // Need to choose the next color via the color_index. 
            // Making sure to choose another color than any of the neighbour blocks by incrementing 
            // the color_index and retrying if the color is the same as any neighbour
            unsigned int chosen_color = colors[color_index % 4];

            // If the row is more than 0, it means that we have created a neighbour block above the current block.
            // If the color we chose is equal to the color of that neighbour, we try the next color in the colors array
            if (row > 0 && blocks[blockIndex - n_cols].color == chosen_color) {
                color_index++;
                chosen_color = colors[color_index % 4];
            }
            // If the column is more than 0, it means that we have created a neighbour block besides the current block.
            // If the color we chose is equal to the color of that neighbour, we try the next color in the colors array
            if (col > 0 && blocks[blockIndex - 1].color == chosen_color) {
                color_index++;
                chosen_color = colors[color_index % 4];
            }
            // Since we max need to change the color 2 times, there will always be a possible color to choose

            // Setting the block color to the final chosen color
            blocks[blockIndex].color = chosen_color;

            // Drawing the actual block in the VGA
            DrawBlock(width - (col + 1) * 15, row * 15, 15 - 1, 15 - 1, chosen_color);

            // Incrementing the indexes to use on the next block
            blockIndex++;
            color_index++;
        }
    }
}


void play()
{
    ClearScreen();
    initialize_blocks();
    draw_ball();
    DrawBar(bar.middle_pos_y - 23);
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
        // Added wait call to limit the speed of the ball
        wait();
        draw_playing_field();
        draw_ball();
        DrawBar(bar.middle_pos_y - 23); // Finding the y coordinate of the top pixels of the bar
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
    // Resetting the coordinates of the bar and ball
    bar.middle_pos_y = 112;
    ball.middle_pos_x = 11;
    ball.middle_pos_y = 120;
    ball.degrees = 90;
    ball.middle_pos_x_old = 11;
    ball.middle_pos_y_old = 120;

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
    
    // Adding all the destroyed blocks back
    for (int i = 0; i < 16 * n_cols; i++) {
        blocks[i].destroyed = 0;
        blocks[i].deleted = 0;
    }
}

void wait_for_start()
{
    int pressed_key = 0;
    // TODO: Implement waiting behaviour until the user presses either w/s
    unsigned char char_received = 0;

    // Writing initial message to player
    write("Press w or s to start \n");

    // Checking the UART buffer until the user presses w or s
    while (pressed_key != 1) {
        // Reading the next byte from UART buffer
        unsigned long long out = ReadUart();

        // Checking if the UART buffer is ready (MSB 0x8000 flag set)
        if (!(out & 0x8000)) {
            continue; // If not ready, continue waiting
        }

        // Getting the lower 8 bits of the data
        char_received = out & 0xFF;

        // Checking if the bits correspond to w or s
        if (char_received == 0x77 || char_received == 0x73) {
            // If true, then breaking the loop and returning to start the game
            pressed_key = 1;
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    // Clearing the screen in the beginning of the game
    ClearScreen();

    // Checking if the number of columns are legal. If not, we print message to the UART window and return
    if (n_cols < 1 || n_cols > 18) {
        write("n_cols out of legal range [1, 18] \n");
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