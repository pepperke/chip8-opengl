#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <GL/freeglut.h>
#include "emulator.h"

// struct timeval tv;
// unsigned count = 0;
// unsigned start, end;

GLenum rcode;

int main(int argc, char *argv[]) {
    reset();
    
    srand(1);

    // gettimeofday(&tv, NULL);
    // start = 1000000 * tv.tv_sec + tv.tv_usec;
    // end = start;
    
    char path[MAX_PATH_LEN + 1];
    memset(path, 0, sizeof(path));

    if (argc < 2) {
        CHIP_EXIT_ERR("Specify a binary program. Usage ./chip8 *bin*");
    }

    BYTE symb_count = 0;
    while (argv[1][symb_count] && symb_count < MAX_PATH_LEN) {
        if (isprint(argv[1][symb_count])) {
            path[symb_count] = argv[1][symb_count];
            symb_count++;
        }
        else
            CHIP_EXIT_ERR("Invalid file path");
    }

    read_program(path);
    write_sprite_data();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(300, 300);
    glutCreateWindow("window");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, 1, -1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GL_EXIT_ERR(rcode, "Init failed");

    glutKeyboardFunc(handleKeys);
    glutDisplayFunc(display);
    glutTimerFunc(1, idle, 0);
    glutTimerFunc(1000/60, decrease_delay, 0);

    glutMainLoop();

    return 0;
}

void decrease_delay(int _val) {
    c.delay_timer -= 1;
    glutTimerFunc(1000/60, decrease_delay, 0);
}

void push_PC_stack() {
    if (c.SP >= 15) {
        CHIP_EXIT_ERR("No space left in stack");
    }
    c.stack[c.SP++] = c.PC;
}

WORD pop_PC_stack() {
    if (c.SP < 1) {
        CHIP_EXIT_ERR("Stack is empty");
    }
    return c.stack[--c.SP];
}

void reset() {
    memset(c.memory, 0, sizeof(c.memory));
    memset(c.stack, 0, sizeof(c.stack));
    memset(c.registers, 0, sizeof(c.registers));
    c.rI = 0x200;
    c.SP = 0;
    c.PC = 0x200;
    c.delay_timer = 255;
    c.sound_timer = 0;
    c.program_size = 0;
    c.key = 0xFF;
}

// Read and execute operation
void read_op(int once) {
    BYTE high = c.memory[c.PC];
    BYTE low  = c.memory[c.PC + 1];
    
    WORD opcode = (high << 8) | low;

    // printf("%04X opcode, %04X PC\n", opcode, c.PC);
    
    BYTE X = high & 0xF;
    BYTE Y = (low >> 4) & 0xF;

    BYTE N   = opcode & 0x00F;
    BYTE NN  = opcode & 0x0FF;
    WORD NNN = opcode & 0xFFF;
    WORD TMP;
    
    BYTE VX = c.registers[X];
    BYTE VY = c.registers[Y];

    switch (opcode >> 12 & 0xF) {
        case 0x0:
            switch (opcode & 0xFFF) {
                case 0x0:
                    CHIP_EXIT_ERR("Reached opcode 0");
                case 0xE0:
                    glClear(GL_COLOR_BUFFER_BIT);
                    glutSwapBuffers();
                    break;
                case 0xEE:
                    c.PC = pop_PC_stack();
                    break;
                case 0xEF:
                    glutSwapBuffers();
                    break;
                default:
                    // unimplemented
                    break;
            }
        break;
        case 0x1:
            c.PC = NNN;
            return;
        case 0x2:
            push_PC_stack();
            c.PC = NNN;
            return;
        case 0x3:
            if (c.registers[X] == NN) {
                c.PC += PC_LEN;
            }
            break;
        case 0x4:
            if (c.registers[X] != NN) {
                c.PC += PC_LEN;
            }
            break;
        case 0x5:
            if (c.registers[X] == VY) {
                c.PC += PC_LEN;
            }
            break;
        case 0x6:
            c.registers[X] = NN;
            break;
        case 0x7:
            c.registers[X] += NN;
            break;
        case 0x8:
            switch (opcode & 0xF) {
                case 0x0:
                    c.registers[X] = VY;
                    break;
                case 0x1:
                    c.registers[X] |= VY;
                    break;
                case 0x2:
                    c.registers[X] &= VY;
                    break;
                case 0x3:
                    c.registers[X] ^= VY;
                    break;
                case 0x4:
                    if (c.registers[X] > 0xFF - VY) {
                        c.registers[0xF] = 1;
                    }
                    else {
                        c.registers[0xF] = 0;
                    }
                    c.registers[X] += VY;
                    break;
                case 0x5:
                    if (c.registers[X] < VY) {
                        c.registers[0xF] = 0;
                    }
                    else {
                        c.registers[0xF] = 1;
                    }
                    c.registers[X] -= VY;
                    break;
                case 0x6:
                    c.registers[0xF] = VX & 0x1;
                    c.registers[X] >>= 1;
                    break;
                case 0x7:
                    if (c.registers[Y] < VX) {
                        c.registers[0xF] = 0;
                    }
                    else {
                        c.registers[0xF] = 1;
                    }
                    c.registers[X] = VY - VX;
                    break;
                case 0xE:
                    c.registers[0xF] = VX & 0x80;
                    c.registers[X] <<= 1;
                    break;
                default:
                    CHIP_EXIT_ERR("No such opcode %04X", opcode);
            }
        break;
        case 0x9:
            if (VX != VY) {
                c.PC += PC_LEN;
            }
            break;
        case 0xA:
            c.rI = NNN;
            break;
        case 0xB:
            c.PC = c.registers[0] + NNN;
            break;
        case 0xC:
            c.registers[X] = (rand() % 255) & NN;
            break;
        case 0xD:
            draw_sprite(VX, VY, N);
            break;
        case 0xE: 
            switch (opcode & 0xFF) {
                case 0x9E:
                    if (c.registers[X] == c.key) {
                        c.PC += PC_LEN;
                    }
                    break;
                case 0xA1:
                    if (c.registers[X] != c.key) {
                        c.PC += PC_LEN;
                    }
                    break;
                default:
                    CHIP_EXIT_ERR("No such opcode %04X", opcode);
            }
        break;
        case 0xF:
            switch (opcode & 0xFF) {
                case 0x7:
                    c.registers[X] = c.delay_timer;
                    break;
                case 0xA:
                    if (c.key == 0xFF) {
                        read_op(1);
                        return;
                    }
                    c.registers[X] = c.key;
                    break;
                case 0x15:
                    c.delay_timer = VX;
                    break;
                case 0x18:
                    c.sound_timer = VX;
                    break;
                case 0x1E:  
                    c.rI += VX;
                    break;
                case 0x29:  // FX29 LDSPR
                    c.rI = c.program_size + CHAR_HEIGHT * VX;
                    break;
                case 0x33:  // FX33 BCD
                    c.memory[c.rI] = VX / 100;
                    VX %= 100;
                    c.memory[c.rI + 1] = VX / 10;
                    c.memory[c.rI + 2] = VX % 10;
                    break;
                case 0x55:
                    for (BYTE k = 0; k <= X; k++) {
                        c.memory[c.rI + k] = c.registers[k];
                    }
                    break;
                case 0x65:  // FX65 READ
                    for (BYTE k = 0; k <= X; k++) {
                        c.registers[k] = c.memory[c.rI + k];
                    }
                    break;
                default:
                    CHIP_EXIT_ERR("No such opcode %04X", opcode);
            }
        break;
        default:
            CHIP_EXIT_ERR("No such opcode %04X", opcode);
    }
    if (once)
        return;

    c.PC += PC_LEN;
}

static int pixel_color(BYTE x, BYTE y) {
    BYTE pixel;
    glReadPixels(x * 10, (HEIGHT - 1) - y * 10, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &pixel);
    GL_EXIT_ERR(rcode, "Couldn't read pixels");
    return pixel;
}

static void draw_line(BYTE memory_byte, BYTE x, BYTE y) {
    glMatrixMode(GL_MODELVIEW);

    for (BYTE i = 0; i < 8; i++) {
        if ((memory_byte >> (7 - i)) & 1) {
            if (pixel_color(x + i, y) != 0) {
                glColor3f(0.f, 0.f, 0.f);
                c.registers[0xF] = 1;
            }
            else 
                glColor3f(1.f, 1.f, 1.f);

            glLoadIdentity();
            glTranslatef((x + i) * 10, y * 10, 0);

            glBegin(GL_QUADS);
                glVertex2f( 0.f,  0.f);
                glVertex2f(10.f,  0.f);
                glVertex2f(10.f, 10.f);
                glVertex2f( 0.f, 10.f);
            glEnd();
        }
    }
}

void draw_sprite(BYTE x, BYTE y, BYTE N) {
    c.registers[0xF] = 0;

    for (BYTE i = 0; i < N; i++) {
        draw_line(c.memory[c.rI + i], x, y + i);
    }
}

void handleKeys(BYTE key_code, int _x, int _y) {
    if (key_code >= '0' && key_code <= '9')
        c.key = key_code - '0';
    else if (key_code >= 'a' && key_code <= 'f')
        c.key = key_code - 'a' + 10;
    else
        c.key = 0xFF;
}

void idle(int _val) {
    for (BYTE i = 0; i < 2; i++) {
        // if (end - start >= 1000000) {
            // start = end;
            // printf("executed %d instructions per sec\n", count);
            // count = 0;
        // }
        read_op(0);
        // count++;
        // gettimeofday(&tv, NULL);
        // end = 1000000 * tv.tv_sec + tv.tv_usec;
    }

    glutTimerFunc(3, idle, 0);
}

void read_program(const char *path) {
    BYTE byte, temp;
    WORD i = 0x200;

    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("Error while opening the file.\n");
        exit(253);
    }
    
    do {
        byte = fgetc(file);
        c.memory[i++] = byte;
    } while (!feof(file) && i < sizeof(c.memory) - 1);

    fclose(file);

    c.program_size = i;
}

void write_sprite_data() {
    if (sizeof(c.memory) - c.program_size < CHAR_HEIGHT * 16) 
        CHIP_EXIT_ERR("No memory for sprite data left");
    
    WORD offset = c.program_size;
    // zero
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;

    // one
    c.memory[offset++] = 0x20;
    c.memory[offset++] = 0x60;
    c.memory[offset++] = 0x20;
    c.memory[offset++] = 0x20;
    c.memory[offset++] = 0x70;

    // two
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;

    // three
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0xF0;

    // four
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0x10;

    // five

    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0xF0;

    // six
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;

    // seven
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0x20;
    c.memory[offset++] = 0x40;
    c.memory[offset++] = 0x40;

    // eight
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;

    // nine
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x10;
    c.memory[offset++] = 0xF0;

    // A
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;

    // B
    c.memory[offset++] = 0xE0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xE0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xE0;

    // C
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;

    // D
    c.memory[offset++] = 0xE0;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0x90;
    c.memory[offset++] = 0xE0;

    // E
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;

    // F
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0xF0;
    c.memory[offset++] = 0x80;
    c.memory[offset++] = 0x80;
}
