#include "gdt.h"
#include "interrupts.h"
#include "graphics.h"
#include "ps2.h"

typedef struct {
    char x[4];
    char y[4];
    unsigned char centerX;
    unsigned char centerY;
    unsigned char color;
} Piece;

extern unsigned char tileData[7][20][20][3];
unsigned char tiles[20][10];

void drawTile(int x, int y, unsigned char color) {
    if (color == 0 || color > 7) {
        rectangle(x*20, y*20, 20, 20, 0x00000000);
        return;
    }
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            plot(x*20+j, y*20+i, tileData[color-1][i][j][0] << 16 | tileData[color-1][i][j][1] << 8 | tileData[color-1][i][j][2]);
        }
    }
}

Piece templatePieces[] = {
    { // I
        .x = {0, 0, 0, 0},
        .y = {-1, 0, 1, 2},
        .centerX = 4,
        .centerY = 1,
        .color = 1
    },
    { // L
        .x = {-1, 0, 1, 1},
        .y = {0, 0, 0, -1},
        .centerX = 4,
        .centerY = 1,
        .color = 2
    },
    { // J
        .x = {1, 0, -1, -1},
        .y = {0, 0, 0, -1},
        .centerX = 4,
        .centerY = 1,
        .color = 3
    },
    { // 0
        .x = {0, 1, 0, 1},
        .y = {0, 0, 1, 1},
        .centerX = 4,
        .centerY = 1,
        .color = 4
    },
    { // T
        .x = {0, -1, 1, 0},
        .y = {0, 0, 0, 1},
        .centerX = 4,
        .centerY = 1,
        .color = 5
    },
    { // Z
        .x = {-1, 0, 0, 1},
        .y = {-1, -1, 0, 0},
        .centerX = 4,
        .centerY = 1,
        .color = 6
    },
    { // S
        .x = {1, 0, 0, -1},
        .y = {-1, -1, 0, 0},
        .centerX = 4,
        .centerY = 1,
        .color = 7
    }
};

Piece fallingPiece;
Piece nextPiece;

int randCounter = 0;
int rand() {
    return randCounter++;
}

void draw() {
    rectangle(0, 0, 400, 400, 0xff555555);
    // draw tile grid
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            drawTile(j, i, tiles[i][j]);
        }
    }
    // draw falling piece
    for (int i = 0; i < 4; i++) {
        drawTile(fallingPiece.centerX + fallingPiece.x[i], fallingPiece.centerY + fallingPiece.y[i], fallingPiece.color);
    }
    rectangle(260, 80, 60, 80, 0);
    // draw next piece
    for (int i = 0; i < 4; i++) {
        drawTile(14 + nextPiece.x[i], 5 + nextPiece.y[i], nextPiece.color);
    }
    //updateWindow(); // we will deal with this later
}

void update() {
    int canFall = 1;
    // check if piece can fall
    for (int i = 0; i < 4; i++) {
        if (fallingPiece.centerY + fallingPiece.y[i] + 1 == 20 || tiles[fallingPiece.centerY + fallingPiece.y[i] + 1][fallingPiece.centerX + fallingPiece.x[i]]) {
            canFall = 0;
            break;
        }
    }
    if (canFall) {
        // if so, move piece down
        fallingPiece.centerY++;
    }
    else {
        // otherwise, snap piece to tile grid
        for (int i = 0; i < 4; i++) {
            tiles[fallingPiece.centerY + fallingPiece.y[i]][fallingPiece.centerX + fallingPiece.x[i]] = fallingPiece.color;
        }
        // and pick new piece
        fallingPiece = nextPiece;
        nextPiece = templatePieces[rand() % 7];
    }
    // check for full lines
    for (int i = 0; i < 20; i++) {
        int lineFull = 1;
        for (int j = 0; j < 10; j++) {
            if (!tiles[i][j]) {
                lineFull = 0;
                break;
            }
        }
        if (lineFull) {
            for (int j = i; j > 0; j--) {
                for (int k = 0; k < 10; k++) {
                    tiles[j][k] = tiles[j-1][k];
                }
            }
        }
    }
}

int extended = 0;

void keypress(unsigned char key) {
    if (key == 0xe0) {
        extended = 1;
        return;
    }
    if (extended) {
    switch (key) {
        case 0x4b:
        fallingPiece.centerX--;
        draw();
        break;
        case 0x4d:
        fallingPiece.centerX++;
        draw();
        break;
        case 0x48:
        for (int i = 0; i < 4; i++) {
            char newX = fallingPiece.y[i];
            fallingPiece.y[i] = -fallingPiece.x[i];
            fallingPiece.x[i] = newX;
        }
        draw();
        break;
        case 0x50:
        update();
        draw();
        break;
    }
    extended = 0;
    }
}

Framebuffer readFramebufferInfo(void* mbInfo) {
    Framebuffer fb;
    fb.data = 0; // if we can't find a framebuffer, data is 0
    unsigned int flags = *(unsigned int*)mbInfo;
    if (flags & (1 << 12)) { // if fb section exists
          unsigned int type = *(unsigned int*)(mbInfo + 109);
          if (type == 2) { // if fb type is not text mode
                return fb;
          }
          fb.data = *(int**)(mbInfo + 88);
          fb.width = *(int*)(mbInfo + 100);
          fb.height = *(int*)(mbInfo + 104);
    }
    return fb;
}

int kmain() {
    int magic;
    void* mbInfo;
    asm("" : "=a" (magic), "=b" (mbInfo):);
    Framebuffer fb = readFramebufferInfo(mbInfo);
    if (fb.data) { // using graphics mode
          setFb(fb);
    }
    else {
        unsigned char message[] = "Refusing to start in text mode!";
        unsigned char* textmem = (unsigned char*) 0xb8000;
        for (int i = 0; message[i] != 0; i++) {
            textmem[i*2] = message[i];
        }
          while(1); // hang, tetris won't work in text mode (yet!)
    }
    initializeGdt();
    initializeIdt();
    init_ps2_1(keypress);
    fallingPiece = templatePieces[rand() % 7];
    nextPiece = templatePieces[rand() % 7];
    int updateTick = 0;
    while(1) {
        if (updateTick == 0) {
            update();
            draw();
        }
        updateTick++;
        if (updateTick == 500000000) {
            updateTick = 0;
        }
    }
}