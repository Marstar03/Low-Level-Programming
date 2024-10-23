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

#define FRAMEBUFFER_DIR "/dev/"
#define TARGET_FB_NAME "RPi-Sense FB"
#define MAX_FB_PATH 256

#define JOYSTICK_NAME "Raspberry Pi Sense HAT Joystick"
#define INPUT_DIR "/dev/input/"

#define COLOR_RED {255, 0, 0}
#define COLOR_GREEN {0, 255, 0}
#define COLOR_BLUE {0, 0, 255}
#define COLOR_YELLOW {255, 255, 0}
#define COLOR_PURPLE {128, 0, 128}


// Global joystick file descriptor
int joystick_fd = -1;

// If you extend this structure, either avoid pointers or adjust
// the game logic allocate/deallocate and reset the memory
typedef struct
{
    bool occupied;
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

static inline bool tileOccupied(coord const target);


// Helper function to convert RGB888 to RGB565
uint16_t rgb888_to_rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    uint16_t r = (red >> 3) & 0x1F;
    uint16_t g = (green >> 2) & 0x3F;
    uint16_t b = (blue >> 3) & 0x1F;
    return (r << 11) | (g << 5) | b;
}

bool find_sensehat_fb(char *fb_path, size_t path_len) {
    struct fb_fix_screeninfo finfo;
    char device_path[MAX_FB_PATH];
    int fb_fd;
    
    // Iterate through /dev/fb0 to /dev/fb31
    for (int i = 0; i < 32; i++) {
        snprintf(device_path, sizeof(device_path), FRAMEBUFFER_DIR "fb%d", i);
        
        // Attempt to open the framebuffer device
        fb_fd = open(device_path, O_RDWR);
        if (fb_fd == -1) {
            // Unable to open, possibly doesn't exist, skip to next
            continue;
        }
        
        // Retrieve fixed screen information
        if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
            perror("ioctl FBIOGET_FSCREENINFO failed");
            close(fb_fd);
            continue;
        }
        
        // Compare the framebuffer name
        if (strncmp(finfo.id, TARGET_FB_NAME, strlen(TARGET_FB_NAME)) == 0) {
            // Found the correct framebuffer
            strncpy(fb_path, device_path, path_len - 1);
            fb_path[path_len - 1] = '\0';  // Ensure null-termination
            close(fb_fd);
            return true;
        }
        
        // Not the target framebuffer, close and continue
        close(fb_fd);
    }
    
    // Framebuffer not found
    return false;
}


// This function is called on the start of your application
// Here you can initialize what ever you need for your task
// return false if something fails, else true
// Initialize Sense HAT and set all LEDs to one color
bool initializeSenseHat(uint16_t **fb_mem, size_t *fb_size, char *fb_path) {
    struct fb_fix_screeninfo finfo;
    int fb_fd;
    
    // Find the correct framebuffer device
    if (!find_sensehat_fb(fb_path, MAX_FB_PATH)) {
        fprintf(stderr, "RPi-Sense FB not found!\n");
        return false;
    }
    
    // Open the framebuffer device
    fb_fd = open(fb_path, O_RDWR);
    if (fb_fd == -1) {
        perror("Error opening framebuffer device");
        return false;
    }
    
    // Retrieve fixed screen information
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("ioctl FBIOGET_FSCREENINFO failed");
        close(fb_fd);
        return false;
    }
    
    // Retrieve variable screen information (needed for memory mapping)
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("ioctl FBIOGET_VSCREENINFO failed");
        close(fb_fd);
        return false;
    }
    
    // Calculate the size of the framebuffer
    *fb_size = finfo.smem_len;
    
    // Memory-map the framebuffer
    *fb_mem = (uint16_t *)mmap(NULL, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (*fb_mem == MAP_FAILED) {
        perror("Error memory-mapping the framebuffer");
        close(fb_fd);
        return false;
    }
    
    // Close the framebuffer file descriptor; the memory remains mapped
    close(fb_fd);
    
    return true;
}


// Set the color of the entire 8x8 LED matrix via memory-mapped framebuffer
void setMatrixColor(uint16_t *fb_mem, uint8_t red, uint8_t green, uint8_t blue) {
    uint16_t color = rgb888_to_rgb565(red, green, blue);
    
    // Set each pixel to the given color
    for (int i = 0; i < 8 * 8; i++) {
        fb_mem[i] = color;
    }
}

void setTileColor(uint16_t *fb_mem, uint8_t red, uint8_t green, uint8_t blue, unsigned int x, unsigned int y) {
    uint16_t color = rgb888_to_rgb565(red, green, blue);
    
    int fb_mem_index = y * 8 + x;
    fb_mem[fb_mem_index] = color;
}

// Unmap the framebuffer when done
void cleanupSenseHat(uint16_t *fb_mem, size_t fb_size) {
    if (munmap(fb_mem, fb_size) == -1) {
        perror("Error unmapping the framebuffer");
    }
}

// This function is called when the application exits
// Here you can free up everything that you might have opened/allocated
void freeSenseHat()
{
}

int find_joystick_device(char *joystick_path, size_t path_len) {
    struct dirent *entry;
    DIR *dp = opendir(INPUT_DIR);

    if (dp == NULL) {
        perror("Error opening input directory");
        return -1;
    }

    // Iterate over each entry in the input directory
    while ((entry = readdir(dp)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {  // Look for event devices
            char device_path[256];
            snprintf(device_path, sizeof(device_path), INPUT_DIR "%s", entry->d_name);

            // Open the input device
            int fd = open(device_path, O_RDONLY);
            if (fd < 0) {
                continue;
            }

            // Get the name of the input device
            char name[256];
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
                close(fd);
                continue;
            }

            // Check if it matches the joystick name
            if (strncmp(name, JOYSTICK_NAME, strlen(JOYSTICK_NAME)) == 0) {
                strncpy(joystick_path, device_path, path_len);
                joystick_path[path_len - 1] = '\0';  // Ensure null termination
                close(fd);
                closedir(dp);
                return 0;  // Success
            }

            // Close the file descriptor and continue
            close(fd);
        }
    }

    closedir(dp);
    return -1;  // Joystick not found
}

int initializeJoystick() {
    char joystick_path[256];

    if (find_joystick_device(joystick_path, sizeof(joystick_path)) == -1) {
        fprintf(stderr, "Joystick not found!\n");
        return -1;
    }

    joystick_fd = open(joystick_path, O_RDONLY | O_NONBLOCK);
    if (joystick_fd == -1) {
        perror("Error opening joystick device");
        return -1;
    }

    return 0;  // Success
}

// Close the joystick file descriptor
void closeJoystick() {
    if (joystick_fd != -1) {
        close(joystick_fd);
    }
}

// This function should return the key that corresponds to the joystick press
// KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, with the respective direction
// and KEY_ENTER, when the the joystick is pressed
// !!! when nothing was pressed you MUST return 0 !!!
int readSenseHatJoystick() {
    if (joystick_fd == -1) {
        printf("No joystick");
        return 0;  // No joystick
    }

    struct pollfd pollJoystick = {
        .fd = joystick_fd,
        .events = POLLIN
    };

    int lkey = 0;

    // Poll for input without blocking
    if (poll(&pollJoystick, 1, 0)) {
        struct input_event ev;
        while (read(joystick_fd, &ev, sizeof(ev)) > 0) {
            if (ev.type == EV_KEY && ev.value == 1) {  // EV_KEY with value 1 means key press
                switch (ev.code) {
                    case KEY_ENTER:
                        return KEY_ENTER;
                    case KEY_UP:
                        return KEY_UP;
                    case KEY_DOWN:
                        return KEY_DOWN;
                    case KEY_RIGHT:
                        return KEY_RIGHT;
                    case KEY_LEFT:
                        return KEY_LEFT;
                }
            }
        }
    }

    return 0;  // No relevant input
}

// This function should render the gamefield on the LED matrix. It is called
// every game tick. The parameter playfieldChanged signals whether the game logic
// has changed the playfield
void renderSenseHatMatrix(uint16_t *fb_mem, bool const playfieldChanged)
{
    //(void)playfieldChanged;

    if (!playfieldChanged) {
        return;
    }

    for (unsigned int y = 0; y < game.grid.y; y++)
    {
        for (unsigned int x = 0; x < game.grid.x; x++)
        {
            coord const checkTile = {x, y};
            if (tileOccupied(checkTile)) {
                uint8_t red = game.playfield[y][x].red;
                uint8_t green = game.playfield[y][x].green;
                uint8_t blue = game.playfield[y][x].blue;
                setTileColor(fb_mem, red, green, blue, x, y);
            } else {
                setTileColor(fb_mem, 0, 0, 0, x, y);
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

    uint8_t colors[][3] = {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_YELLOW,
        COLOR_PURPLE
    };

    // Select a random color index
    int colorIndex = rand() % 5;

    game.playfield[target.y][target.x].red = colors[colorIndex][0];
    game.playfield[target.y][target.x].green = colors[colorIndex][1];
    game.playfield[target.y][target.x].blue = colors[colorIndex][2];
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

    if (!initializeSenseHat(&fb_mem, &fb_size, fb_path))
    {
        fprintf(stderr, "ERROR: could not initilize sense hat\n");
        return 1;
    } 
    if (initializeJoystick() != 0)
    {
        fprintf(stderr, "ERROR: could not initialize joystick\n");
        return 1;
    }

    // Clear console, render first time
    fprintf(stdout, "\033[H\033[J");
    renderConsole(true);
    renderSenseHatMatrix(fb_mem, true);
    setMatrixColor(fb_mem, 0, 0, 0);

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
            key = readKeyboard();
        }
        if (key == KEY_ENTER)
            break;

        bool playfieldChanged = sTetris(key);
        renderConsole(playfieldChanged);
        renderSenseHatMatrix(fb_mem, playfieldChanged);

        // Wait for next tick
        gettimeofday(&eTv, NULL);
        unsigned long const uSecProcessTime = ((eTv.tv_sec * 1000000) + eTv.tv_usec) - ((sTv.tv_sec * 1000000 + sTv.tv_usec));
        if (uSecProcessTime < game.uSecTickTime)
        {
            usleep(game.uSecTickTime - uSecProcessTime);
        }
        game.tick = (game.tick + 1) % game.nextGameTick;
    }

    freeSenseHat();
    cleanupSenseHat(fb_mem, fb_size);
    closeJoystick();
    free(game.playfield);
    free(game.rawPlayfield);

    return 0;
}
