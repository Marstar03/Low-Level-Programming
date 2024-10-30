#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <stdint.h>
#include <fcntl.h>
#include <dirent.h>

// The game state can be used to detect what happens on the playfield
#define GAMEOVER 0
#define ACTIVE (1 << 0)
#define ROW_CLEAR (1 << 1)
#define TILE_ADDED (1 << 2)

#define FRAMEBUFFER_DIR "/dev/" // directory path for searching frame buffers
#define TARGET_FB_NAME "RPi-Sense FB" // name of the target sense hat frame buffer
#define MAX_FB_PATH 256

#define INPUT_DIR "/dev/input/" // directory path for searching input devices
#define JOYSTICK_NAME "Raspberry Pi Sense HAT Joystick" // name of the target sense hat joystick input device

// definition of different colors to choose between when adding a new tetris tile
#define COLOR_RED {255, 0, 0}
#define COLOR_GREEN {0, 255, 0}
#define COLOR_BLUE {0, 0, 255}
#define COLOR_YELLOW {255, 255, 0}
#define COLOR_PURPLE {128, 0, 128}


// global joystick file descriptor
int joystick_fd = -1;

// If you extend this structure, either avoid pointers or adjust
// the game logic allocate/deallocate and reset the memory
typedef struct
{
    bool occupied;

    // added colors for each tile in order to separate them on the sense hat
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} tile;

typedef struct
{
    unsigned int x;
    unsigned int y;
} coord;

typedef struct
{
    coord const grid;                     // playfield bounds
    unsigned long const uSecTickTime;     // tick rate
    unsigned long const rowsPerLevel;     // speed up after clearing rows
    unsigned long const initNextGameTick; // initial value of nextGameTick

    unsigned int tiles; // number of tiles played
    unsigned int rows;  // number of rows cleared
    unsigned int score; // game score
    unsigned int level; // game level

    tile *rawPlayfield; // pointer to raw memory of the playfield
    tile **playfield;   // This is the play field array
    unsigned int state;
    coord activeTile; // current tile

    unsigned long tick;         // incremeted at tickrate, wraps at nextGameTick
                                // when reached 0, next game state calculated
    unsigned long nextGameTick; // sets when tick is wrapping back to zero
                                // lowers with increasing level, never reaches 0
} gameConfig;

gameConfig game = {
    .grid = {8, 8},
    .uSecTickTime = 10000,
    .rowsPerLevel = 2,
    .initNextGameTick = 50,
};

// forward declarations for functions that are being called before they are defined
static inline bool tileOccupied(coord const target);


// helper function to convert color values from 8 bits each to one 16 bit int
uint16_t rgb888_to_rgb565(uint8_t red, uint8_t green, uint8_t blue) 
{
    uint16_t r = (red >> 3) & 0x1F; // reducing red from 8 to 5 bits by right shifting 3 bits and then only keeping the remaining 5 bits
    uint16_t g = (green >> 2) & 0x3F; // reducing green from 8 to 6 bits by right shifting 2 bits
    uint16_t b = (blue >> 3) & 0x1F; // reducing blue from 8 to 5 bits by right shifting 3 bits
    return (r << 11) | (g << 5) | b; // returning the concatenation of the resulting rgb values
}

// function for finding the path to the correct sense hat led matrix frame buffer
bool find_sensehat_fb(char *fb_path, size_t path_len) 
{
    struct fb_fix_screeninfo finfo; // struct for holding the sense hat screen information fetched using the frame buffer device api
    char device_path[MAX_FB_PATH]; // will hold the path to the frame buffer we are checking
    int fb_fd; // file descriptor for the frame buffer
    
    // iterating over all possible frame buffers, which is at most 32. Want to find the one for the sense hat by checking the name
    for (int i = 0; i < 32; i++) 
    {
        snprintf(device_path, sizeof(device_path), FRAMEBUFFER_DIR "fb%d", i); // creating the file path to the frame buffer we want to check
        
        fb_fd = open(device_path, O_RDWR); // trying to open the frame buffer
        if (fb_fd == -1) // if the file descriptor is -1, it means we were not able to open, so we skip to the next frame buffer
        {
            continue;
        }
        
        if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1) // trying to fetch the screen information using ioctl and copying that to the address of the finfo struct
        {
            // if it returns -1, it failed so we close the file descriptor and try the next frame buffer
            perror("ioctl FBIOGET_FSCREENINFO failed");
            close(fb_fd);
            continue;
        }
        
        if (strncmp(finfo.id, TARGET_FB_NAME, strlen(TARGET_FB_NAME)) == 0) // comparing the id field (name) of the screen info struct with the wanted name
        {
            strncpy(fb_path, device_path, path_len - 1); // copying the device path to the frame buffer path
            fb_path[path_len - 1] = '\0';  // adding null termination
            close(fb_fd); // closing the file descriptor
            return true;
        }

        close(fb_fd); // if the id was not the correct one, we close the file descriptor and keep iterating over the frame buffers
    }
    
    return false; // returning false if we didnt find any matching frame buffers
}

// function for finding the joystick device
bool find_joystick_device(char *joystick_path, size_t path_len) 
{
    struct dirent *entry;
    DIR *dp = opendir(INPUT_DIR);

    if (dp == NULL) // throwing error if not able to open the directory for the input devices, which happens opendir returns null
    {
        perror("Error opening input directory");
        return false;
    }

    // iterating over each of the entries in the input directory. Using while loop compared to for loop when finding the sense hat since we dont know how many entries there are
    while ((entry = readdir(dp)) != NULL) 
    {
        if (strncmp(entry->d_name, "event", 5) == 0) // checking if the entry is an event device
        {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), INPUT_DIR "%s", entry->d_name); // creating the path to the specific event device

            int fd = open(device_path, O_RDONLY); // opening the input device
            if (fd < 0) // checking if the file descriptor is negative. If so, it means it failed to open so we skip this event device
            {
                continue;
            }

            char name[256];
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) == -1) // trying to fetch the name of the input device at the fd file descriptor
            {
                close(fd); // if it failed, we close the file descriptor and continue trying the next event device
                continue;
            }

            if (strncmp(name, JOYSTICK_NAME, strlen(JOYSTICK_NAME)) == 0) // checking if the name we fetched matches the joystick name
            {
                strncpy(joystick_path, device_path, path_len); // if there is a match, we copy the device path to the joystick path
                joystick_path[path_len - 1] = '\0';  // adding null termination
                close(fd); // closing the file descriptor
                closedir(dp); // closing the input directory
                return true;
            }

            close(fd); // if the names didnt match, we close the file descriptor and try another event device
        }
    }

    // if we didnt find any matching event device, we close the input directory and return -1 since the joystick wasnt found
    closedir(dp);
    return false;
}


// This function is called on the start of your application
// Here you can initialize what ever you need for your task
// return false if something fails, else true

// initializing the sense hat led matrix and joystick
bool initializeSenseHat(uint16_t **fb_mem, size_t *fb_size, char *fb_path) 
{
    // first initializing setting up the sense hat led matrix

    struct fb_fix_screeninfo finfo;
    int fb_fd;
    
    if (!find_sensehat_fb(fb_path, MAX_FB_PATH)) // first checking if we find the path to the frame buffer of the sense hat
    {
        fprintf(stderr, "RPi-Sense FB not found\n");
        return false; // returning false if we didnt find it
    }
    
    fb_fd = open(fb_path, O_RDWR); // trying to open the frame buffer device
    if (fb_fd == -1) 
    {
        perror("Error opening framebuffer device");
        return false; // returning false if we couldnt open it
    }
    
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1) // again trying to fetch the screen information using ioctl and copying that to the address of the finfo struct
    {
        // if it returns -1, it failed so we close the file descriptor and return false
        perror("ioctl FBIOGET_FSCREENINFO failed");
        close(fb_fd);
        return false;
    }
    
    *fb_size = finfo.smem_len; // finding the size of the framebuffer
    
    // memory mapping the sense hat frame buffer and setting the fb_mem pointer to point to the start of the frame buffer memory
    // setting first argument to null to let os choose location in memory
    // setting second argument to the size of the framebuffer in memory, and says how much memory to map
    // using the third argument to give read and write permission to the mapped memory, so that we can update the sense hat
    *fb_mem = (uint16_t *)mmap(NULL, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (*fb_mem == MAP_FAILED) // checking if the mapping to the pointer failed by comparing with MAP_FAILED return value
    {
        // if it failed we need to close the file descriptor and return false
        perror("Error memory-mapping the framebuffer");
        close(fb_fd);
        return false;
    }

    close(fb_fd); // closing the framebuffer file descriptor

    // then setting up the sense hat joystick

    char joystick_path[256]; // char array for storing the path of the joystick device

    if (!find_joystick_device(joystick_path, sizeof(joystick_path))) // trying to find the joystick using the function above
    {
        fprintf(stderr, "Joystick not found\n");
        return false; // returning false if it wasnt found
    }

    joystick_fd = open(joystick_path, O_RDONLY | O_NONBLOCK); // trying to open the joystick device in readonly and with nonblock so that it doesnt wait and block for input
    if (joystick_fd == -1) // if the file descriptor is -1, it failed so we return with a failure
    {
        perror("Error opening joystick device");
        return false;
    }
    
    return true; // returning true if everything went ok
}

// function that takes in rgb values in 8 bit format and sets the whole led matrix to that color
void setMatrixColor(uint16_t *fb_mem, uint8_t red, uint8_t green, uint8_t blue) 
{
    uint16_t color = rgb888_to_rgb565(red, green, blue); // calling the function to convert the 8 bit rgb values to a single 16 bit color int on the 565 rgb format
    
    // setting each of the 64 led lights to the retrieved color value
    for (int i = 0; i < 8 * 8; i++) 
    {
        fb_mem[i] = color; // using the pointer we got from the mmap and indexing into each led position to set the color
    }
}

// function that takes in rgb values in 8 bit format and sets a led light at specific xy coordinates in the matrix to that color 
void setTileColor(uint16_t *fb_mem, uint8_t red, uint8_t green, uint8_t blue, unsigned int x, unsigned int y) 
{
    uint16_t color = rgb888_to_rgb565(red, green, blue); // calling the function to convert the 8 bit rgb values to a single 16 bit color int on the 565 rgb format
    
    int fb_mem_index = y * 8 + x; // finding the index into the frame buffer based on the coordinates
    fb_mem[fb_mem_index] = color; // setting the value at that index to the color we found
}

// This function is called when the application exits
// Here you can free up everything that you might have opened/allocated
void freeSenseHat(uint16_t *fb_mem, size_t fb_size)
{
    // want to unmap the sense hat framebuffer when done
    if (munmap(fb_mem, fb_size) == -1) // trying to unmap the framebuffer
    {
        perror("Error unmapping the framebuffer"); // throwing error if it failed
    }

    // when done we also want to close the joystick file descriptor
    if (joystick_fd != -1) // checking if the joystick file descriptor is valid or not
    {
        close(joystick_fd); // closing the file descriptor if it is valid
    }
}

// This function should return the key that corresponds to the joystick press
// KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, with the respective direction
// and KEY_ENTER, when the the joystick is pressed
// !!! when nothing was pressed you MUST return 0 !!!
int readSenseHatJoystick() 
{
    if (joystick_fd == -1) // first checking if the joystick initialization was successful or not by checking the file descriptor
    {
        printf("No joystick");
        return 0;  // didnt find the joystick
    }

    struct pollfd pollJoystick = { // creating a pollfd struct with the joystick file descriptor to use when polling for events
        .fd = joystick_fd, // setting fd as the joystick fd
        .events = POLLIN // using pollin to read data
    };
    
    if (poll(&pollJoystick, 1, 0)) // checking if there is any new input on the file descriptor. Setting the third timeout parameter to 0 to make it nonblocking. So it doesnt wait for any events if there are none
    {
        struct input_event ev; // declaring input event struct as the destination when reading from the file descriptor
        if (read(joystick_fd, &ev, sizeof(ev)) > 0) // reading from the file descriptor and checking if it was successful
        {
            if (ev.type == EV_KEY && (ev.value == 1 || ev.value == 2)) // checking that the event was a key press and if it either was a short press or a long press
            {
                return ev.code; // if that was the case, we return the code which is the value for the specific key that was pressed / joystick direction
            }
        }
    }

    return 0;  // returning 0 if there were no inputs from the joystick
}

// This function should render the gamefield on the LED matrix. It is called
// every game tick. The parameter playfieldChanged signals whether the game logic
// has changed the playfield
void renderSenseHatMatrix(uint16_t *fb_mem, bool const playfieldChanged)
{
    if (!playfieldChanged) // first checking if the game logic has changed the playfield
    {
        return; // if it hasnt changed, we can return immediately
    }

    setMatrixColor(fb_mem, 0, 0, 0); // first setting all the led lights in the matrix to dark

    // double for loop for updating each led light in the matrix to match the updated state of the playfield
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            coord const checkTile = {x, y}; // coordinates of the tile we want to update
            if (tileOccupied(checkTile)) // first checking if the tile has become occupied
            {
                // if it has, we need to update the color of that tile in the matrix to match the color values of the tile in the playfield
                uint8_t red = game.playfield[y][x].red; // getting red value
                uint8_t green = game.playfield[y][x].green; // getting green value
                uint8_t blue = game.playfield[y][x].blue; // getting blue value
                setTileColor(fb_mem, red, green, blue, x, y); // setting the color of the tile
            } 
        }
    }
}

// The game logic uses only the following functions to interact with the playfield.
// if you choose to change the playfield or the tile structure, you might need to
// adjust this game logic <> playfield interface

static inline void newTile(coord const target)
{
    game.playfield[target.y][target.x].occupied = true;

    // added functionality for choosing a random color for the new tile
    uint8_t colors[][3] = { // array containing the possible colors, where each color is defined on the top as an array with 3 elements on the form {red, green, blue}
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_YELLOW,
        COLOR_PURPLE
    };

    // Select a random color index
    int colorIndex = rand() % 5; // selecting a random number from 0 to 4 as index into the color array

    game.playfield[target.y][target.x].red = colors[colorIndex][0]; // setting the red value
    game.playfield[target.y][target.x].green = colors[colorIndex][1]; // setting the green value
    game.playfield[target.y][target.x].blue = colors[colorIndex][2]; // setting the blue value
}

static inline void copyTile(coord const to, coord const from)
{
    memcpy((void *)&game.playfield[to.y][to.x], (void *)&game.playfield[from.y][from.x], sizeof(tile));
}

static inline void copyRow(unsigned int const to, unsigned int const from)
{
    memcpy((void *)&game.playfield[to][0], (void *)&game.playfield[from][0], sizeof(tile) * game.grid.x);
}

static inline void resetTile(coord const target)
{
    memset((void *)&game.playfield[target.y][target.x], 0, sizeof(tile));
}

static inline void resetRow(unsigned int const target)
{
    memset((void *)&game.playfield[target][0], 0, sizeof(tile) * game.grid.x);
}

static inline bool tileOccupied(coord const target)
{
    return game.playfield[target.y][target.x].occupied;
}

static inline bool rowOccupied(unsigned int const target)
{
    for (unsigned int x = 0; x < game.grid.x; x++)
    {
        coord const checkTile = {x, target};
        if (!tileOccupied(checkTile))
        {
            return false;
        }
    }
    return true;
}

static inline void resetPlayfield()
{
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        resetRow(y);
    }
}

// Below here comes the game logic. Keep in mind: You are not allowed to change how the game works!
// that means no changes are necessary below this line! And if you choose to change something
// keep it compatible with what was provided to you!

bool addNewTile()
{
    game.activeTile.y = 0;
    game.activeTile.x = (game.grid.x - 1) / 2;
    if (tileOccupied(game.activeTile))
        return false;
    newTile(game.activeTile);
    return true;
}

bool moveRight()
{
    coord const newTile = {game.activeTile.x + 1, game.activeTile.y};
    if (game.activeTile.x < (game.grid.x - 1) && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool moveLeft()
{
    coord const newTile = {game.activeTile.x - 1, game.activeTile.y};
    if (game.activeTile.x > 0 && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool moveDown()
{
    coord const newTile = {game.activeTile.x, game.activeTile.y + 1};
    if (game.activeTile.y < (game.grid.y - 1) && !tileOccupied(newTile))
    {
        copyTile(newTile, game.activeTile);
        resetTile(game.activeTile);
        game.activeTile = newTile;
        return true;
    }
    return false;
}

bool clearRow()
{
    if (rowOccupied(game.grid.y - 1))
    {
        for (unsigned int y = game.grid.y - 1; y > 0; y--)
        {
            copyRow(y, y - 1);
        }
        resetRow(0);
        return true;
    }
    return false;
}

void advanceLevel()
{
    game.level++;
    switch (game.nextGameTick)
    {
    case 1:
        break;
    case 2 ... 10:
        game.nextGameTick--;
        break;
    case 11 ... 20:
        game.nextGameTick -= 2;
        break;
    default:
        game.nextGameTick -= 10;
    }
}

void newGame()
{
    game.state = ACTIVE;
    game.tiles = 0;
    game.rows = 0;
    game.score = 0;
    game.tick = 0;
    game.level = 0;
    resetPlayfield();
}

void gameOver()
{
    game.state = GAMEOVER;
    game.nextGameTick = game.initNextGameTick;
}

bool sTetris(int const key)
{
    bool playfieldChanged = false;

    if (game.state & ACTIVE)
    {
        // Move the current tile
        if (key)
        {
            playfieldChanged = true;
            switch (key)
            {
            case KEY_LEFT:
                moveLeft();
                break;
            case KEY_RIGHT:
                moveRight();
                break;
            case KEY_DOWN:
                while (moveDown())
                {
                };
                game.tick = 0;
                break;
            default:
                playfieldChanged = false;
            }
        }

        // If we have reached a tick to update the game
        if (game.tick == 0)
        {
            // We communicate the row clear and tile add over the game state
            // clear these bits if they were set before
            game.state &= ~(ROW_CLEAR | TILE_ADDED);

            playfieldChanged = true;
            // Clear row if possible
            if (clearRow())
            {
                game.state |= ROW_CLEAR;
                game.rows++;
                game.score += game.level + 1;
                if ((game.rows % game.rowsPerLevel) == 0)
                {
                    advanceLevel();
                }
            }

            // if there is no current tile or we cannot move it down,
            // add a new one. If not possible, game over.
            if (!tileOccupied(game.activeTile) || !moveDown())
            {
                if (addNewTile())
                {
                    game.state |= TILE_ADDED;
                    game.tiles++;
                }
                else
                {
                    gameOver();
                }
            }
        }
    }

    // Press any key to start a new game
    if ((game.state == GAMEOVER) && key)
    {
        playfieldChanged = true;
        newGame();
        addNewTile();
        game.state |= TILE_ADDED;
        game.tiles++;
    }

    return playfieldChanged;
}

int readKeyboard()
{
    struct pollfd pollStdin = {
        .fd = STDIN_FILENO,
        .events = POLLIN};
    int lkey = 0;

    if (poll(&pollStdin, 1, 0))
    {
        lkey = fgetc(stdin);
        if (lkey != 27)
            goto exit;
        lkey = fgetc(stdin);
        if (lkey != 91)
            goto exit;
        lkey = fgetc(stdin);
    }
exit:
    switch (lkey)
    {
    case 10:
        return KEY_ENTER;
    case 65:
        return KEY_UP;
    case 66:
        return KEY_DOWN;
    case 67:
        return KEY_RIGHT;
    case 68:
        return KEY_LEFT;
    }
    return 0;
}

void renderConsole(bool const playfieldChanged)
{
    if (!playfieldChanged)
        return;

    // Goto beginning of console
    fprintf(stdout, "\033[%d;%dH", 0, 0);
    for (unsigned int x = 0; x < game.grid.x + 2; x++)
    {
        fprintf(stdout, "-");
    }
    fprintf(stdout, "\n");
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        fprintf(stdout, "|");
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            coord const checkTile = {x, y};
            fprintf(stdout, "%c", (tileOccupied(checkTile)) ? '#' : ' ');
        }
        switch (y)
        {
        case 0:
            fprintf(stdout, "| Tiles: %10u\n", game.tiles);
            break;
        case 1:
            fprintf(stdout, "| Rows:  %10u\n", game.rows);
            break;
        case 2:
            fprintf(stdout, "| Score: %10u\n", game.score);
            break;
        case 4:
            fprintf(stdout, "| Level: %10u\n", game.level);
            break;
        case 7:
            fprintf(stdout, "| %17s\n", (game.state == GAMEOVER) ? "Game Over" : "");
            break;
        default:
            fprintf(stdout, "|\n");
        }
    }
    for (unsigned int x = 0; x < game.grid.x + 2; x++)
    {
        fprintf(stdout, "-");
    }
    fflush(stdout);
}

inline unsigned long uSecFromTimespec(struct timespec const ts)
{
    return ((ts.tv_sec * 1000000) + (ts.tv_nsec / 1000));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    // This sets the stdin in a special state where each
    // keyboard press is directly flushed to the stdin and additionally
    // not outputted to the stdout
    {
        struct termios ttystate;
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag &= ~(ICANON | ECHO);
        ttystate.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    }

    // Allocate the playing field structure
    game.rawPlayfield = (tile *)malloc(game.grid.x * game.grid.y * sizeof(tile));
    game.playfield = (tile **)malloc(game.grid.y * sizeof(tile *));
    if (!game.playfield || !game.rawPlayfield)
    {
        fprintf(stderr, "ERROR: could not allocate playfield\n");
        return 1;
    }
    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        game.playfield[y] = &(game.rawPlayfield[y * game.grid.x]);
    }

    // Reset playfield to make it empty
    resetPlayfield();
    // Start with gameOver
    gameOver();

    uint16_t *fb_mem;
    size_t fb_size;
    char fb_path[MAX_FB_PATH];

    // Change: added inputs to initializeSenseHat function
    if (!initializeSenseHat(&fb_mem, &fb_size, fb_path))
    {
        fprintf(stderr, "ERROR: could not initilize sense hat\n");
        return 1;
    } 

    // Clear console, render first time
    fprintf(stdout, "\033[H\033[J");
    renderConsole(true);
    renderSenseHatMatrix(fb_mem, true); // Change: added fb_mem as parameter

    while (true)
    {
        struct timeval sTv, eTv;
        gettimeofday(&sTv, NULL);

        int key = readSenseHatJoystick();
        if (!key)
        {
            // NOTE: Uncomment the next line if you want to test your implementation with
            // reading the inputs from stdin. However, we expect you to read the inputs directly
            // from the input device and not from stdin (you should implement the readSenseHatJoystick
            // method).
            key = readKeyboard(); // Change: uncommented this line. Make it a comment later
        }
        if (key == KEY_ENTER)
            break;

        bool playfieldChanged = sTetris(key);
        renderConsole(playfieldChanged);
        renderSenseHatMatrix(fb_mem, playfieldChanged); // Change: added fb_mem as parameter

        // Wait for next tick
        gettimeofday(&eTv, NULL);
        unsigned long const uSecProcessTime = ((eTv.tv_sec * 1000000) + eTv.tv_usec) - ((sTv.tv_sec * 1000000 + sTv.tv_usec));
        if (uSecProcessTime < game.uSecTickTime)
        {
            usleep(game.uSecTickTime - uSecProcessTime);
        }
        game.tick = (game.tick + 1) % game.nextGameTick;
    }

    freeSenseHat(fb_mem, fb_size); // Change: added fb_mem and fb_size as parameters
    free(game.playfield);
    free(game.rawPlayfield);

    return 0;
}
