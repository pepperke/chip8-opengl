#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>

#define CHIP_EXIT_ERR(x, ...) { fprintf(stderr, x "\n", ##__VA_ARGS__); exit(255); } // ## before __VA_ARGS__
                                                                                     // means to optionally ignore this arg                                                                                    
#define GL_EXIT_ERR(rcode, x) {                           \
    if ((rcode = glGetError()) != GL_NO_ERROR) {          \
        fprintf(stderr, x "\n%s\n", gluErrorString(rcode)); \
        exit(254);                                        \
    }                                                     \
}

#define CHAR_HEIGHT 5

typedef unsigned short WORD;
typedef unsigned char BYTE;
struct chip8 {
    BYTE display[64*32];
    BYTE memory[4096];
    WORD stack[16];
    BYTE registers[16];
    WORD rI;
    BYTE SP;
    WORD PC;
    BYTE delay_timer;
    BYTE sound_timer;
    WORD program_size;
    BYTE key;
} c;

const WORD WIDTH = 640, HEIGHT = 320, FPS = 60, SPF = 1000 / FPS;
const BYTE MAX_PATH_LEN = 255;
const BYTE PC_LEN = sizeof(c.PC);

void decrease_delay(int);

void push_PC_stack();

WORD pop_PC_stack();

void reset();

void read_op(int _val);

void draw_sprite(BYTE x, BYTE y, BYTE N);

void handleKeys(BYTE key_code, int x, int y);

void display() {}

void idle(int);

void read_program(const char *path);

void write_sprite_data();