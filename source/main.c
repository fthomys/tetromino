#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "piece.h"
#include <stdbool.h>


#define BLOCK_SIZE 30
#define WIDTH 10
#define HEIGHT 20
#define INFO_WIDTH 10
#define SCREEN_WIDTH (BLOCK_SIZE * WIDTH)
#define SCREEN_HEIGHT (BLOCK_SIZE * HEIGHT)

SDL_Window* window;
SDL_Renderer* renderer;
//Mix_Music* music;
TTF_Font* font = NULL;
//Mix_Chunk* soundFix = NULL;
//Mix_Chunk* soundLine = NULL;


int grid[HEIGHT][WIDTH] = {0};
int currentPiece[4][4];
int currentPiecePosX = WIDTH / 2 - 2, currentPiecePosY = 0;
int currentPieceColor;



typedef enum {
    MENU,
    PLAYING,
    GAME_OVER
} GameState;

GameState gameState = MENU;

int recentScores[5] = {0};
int recentIndex = 0;

int menuSelection = 0;


bool initialize();
void drawGrid();
void drawPiece();
bool checkCollision(int x, int y, int shape[4][4]);
void rotatePiece();
void movePieceDown();
void movePieceLeft();
void movePieceRight();
void fixPiece();
void clearLines();
bool gameOver();
void handleEvents(SDL_Event event);
void render();
void drawText(const char* text, int x, int y, SDL_Color color);
void drawGameOver();
void drawBorder();
void drawMenu();
void handleMenuInput(SDL_Event event, bool* quit);





void drawMenu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    drawText("TETROMINO", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 100, white);
    drawText(menuSelection == 0 ? "> Start" : "  Start", SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 - 30, menuSelection == 0 ? yellow : white);
    drawText(menuSelection == 1 ? "> Quit"  : "  Quit",  SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 + 10, menuSelection == 1 ? yellow : white);

    SDL_RenderPresent(renderer);
}




int score = 0;
int level = 1;
int linesClearedTotal = 0;

Uint32 lastFallTime = 0;
Uint32 fallDelay = 500;

void handleGameOverInput(SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        gameState = MENU;
    }
}


void drawGameOver() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Color red = {255, 50, 50, 255};
    drawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, red);
    drawText("Press any key to return to Menu", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 10, red);
    SDL_RenderPresent(renderer);
}

bool gameOver() {
    for (int x = 0; x < WIDTH; x++) {
        if (grid[0][x] != 0) {
            drawGameOver();
            recentScores[recentIndex % 5] = score;
            recentIndex++;
            gameState = GAME_OVER;
            return true;
        }
    }
    return false;
}

void blinkLines(int* linesToClear, int count) {
    for (int i = 0; i < 3; i++) {

        for (int j = 0; j < count; j++) {
            int y = linesToClear[j];
            for (int x = 0; x < WIDTH; x++) {
                grid[y][x] = (i % 2 == 0) ? 0 : 8;
            }
        }
        render();
        SDL_Delay(100);
    }
}



void drawBlock(int x, int y, int colorIndex) {
    SDL_Color base = colors[colorIndex];
    int px = x * BLOCK_SIZE;
    int py = y * BLOCK_SIZE;

    int bandHeight = BLOCK_SIZE / 5;

    SDL_SetRenderDrawColor(renderer,
        (Uint8)(base.r + (255 - base.r) * 0.9),
        (Uint8)(base.g + (255 - base.g) * 0.9),
        (Uint8)(base.b + (255 - base.b) * 0.9), 255);
    SDL_Rect top = {px, py, BLOCK_SIZE, bandHeight};
    SDL_RenderFillRect(renderer, &top);

    SDL_SetRenderDrawColor(renderer,
        (Uint8)(base.r + (255 - base.r) * 0.4),
        (Uint8)(base.g + (255 - base.g) * 0.4),
        (Uint8)(base.b + (255 - base.b) * 0.4), 255);
    SDL_Rect topMid = {px, py + bandHeight, BLOCK_SIZE, bandHeight};
    SDL_RenderFillRect(renderer, &topMid);

    SDL_SetRenderDrawColor(renderer, base.r, base.g, base.b, 255);
    SDL_Rect mid = {px, py + bandHeight * 2, BLOCK_SIZE, bandHeight};
    SDL_RenderFillRect(renderer, &mid);

    SDL_SetRenderDrawColor(renderer,
        (Uint8)(base.r * 0.8),
        (Uint8)(base.g * 0.8),
        (Uint8)(base.b * 0.8), 255);
    SDL_Rect bottomMid = {px, py + bandHeight * 3, BLOCK_SIZE, bandHeight};
    SDL_RenderFillRect(renderer, &bottomMid);

    SDL_SetRenderDrawColor(renderer,
        (Uint8)(base.r * 0.5),
        (Uint8)(base.g * 0.5),
        (Uint8)(base.b * 0.5), 255);
    SDL_Rect bottom = {px, py + bandHeight * 4, BLOCK_SIZE, BLOCK_SIZE - bandHeight * 4};
    SDL_RenderFillRect(renderer, &bottom);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect outline = {px, py, BLOCK_SIZE, BLOCK_SIZE};
    SDL_RenderDrawRect(renderer, &outline);
}






void drawBorder() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect border = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderDrawRect(renderer, &border);
}

void handleMenuInput(SDL_Event event, bool* quit) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w:
                menuSelection = (menuSelection + 1) % 2;
                break;
            case SDLK_DOWN:
            case SDLK_s:
                menuSelection = (menuSelection + 1) % 2;
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                if (menuSelection == 0) {
                    // Reset game state
                    score = 0;
                    level = 1;
                    linesClearedTotal = 0;
                    memset(grid, 0, sizeof(grid));
                    currentPieceColor = rand() % 7 + 1;
                    for (int y = 0; y < 4; y++)
                        for (int x = 0; x < 4; x++)
                            currentPiece[y][x] = shapes[currentPieceColor - 1][y][x];
                    currentPiecePosX = WIDTH / 2 - 2;
                    currentPiecePosY = 0;
                    gameState = PLAYING;
                } else if (menuSelection == 1) {
                    *quit = true;
                }
                break;
        }
    }
}


void clearLines() {
    int linesToClear[4];
    int count = 0;

    for (int y = 0; y < HEIGHT; y++) {
        bool full = true;
        for (int x = 0; x < WIDTH; x++) {
            if (grid[y][x] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            linesToClear[count++] = y;
        }
    }

    if (count > 0) {
        blinkLines(linesToClear, count);
    }

    for (int i = 0; i < count; i++) {
        int line = linesToClear[i];

        for (int y = line; y > 0; y--) {
            for (int x = 0; x < WIDTH; x++) {
                grid[y][x] = grid[y - 1][x];
            }
        }

        for (int x = 0; x < WIDTH; x++) {
            grid[0][x] = 0;
        }
    }

    switch (count) {
        case 1: score += 40 * level; break;
        case 2: score += 100 * level; break;
        case 3: score += 300 * level; break;
        case 4: score += 1200 * level; break;
    }

    linesClearedTotal += count;
    if (linesClearedTotal / 10 + 1 > level) {
        level = linesClearedTotal / 10 + 1;
        fallDelay = fallDelay > 100 ? fallDelay - 50 : fallDelay;
    }
}




int main() {
    printf("Hello World!\n");
    SDL_Event event;
    bool quit = false;

    if (!initialize()) {
        printf("Failed to initialize!\n");
        return -1;
    }

    srand(SDL_GetTicks());
    currentPieceColor = rand() % 7 + 1;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentPiece[y][x] = shapes[currentPieceColor - 1][y][x];
        }
    }

    Uint32 lastFallTime = SDL_GetTicks();
    const Uint32 fallDelay = 500;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;

            switch (gameState) {
                case MENU:
                    handleMenuInput(event, &quit);
                    break;
                case PLAYING:
                    handleEvents(event);
                    break;
                case GAME_OVER:
                    handleGameOverInput(event);
                    break;
            }
        }

        if (gameState == PLAYING) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastFallTime > fallDelay) {
                movePieceDown();
                lastFallTime = currentTime;
            }
        }

        switch (gameState) {
            case MENU:
                drawMenu();
                break;
            case PLAYING:
                render();
                break;
            case GAME_OVER:
                drawGameOver();
                break;
        }

        SDL_Delay(16);
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}



bool initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        printf("TTF Init failed: %s\n", TTF_GetError());
        return false;
    }

    /*
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Init failed: %s\n", Mix_GetError());
        return false;
    }
*/
    window = SDL_CreateWindow("TETROMINO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT + 60, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    font = TTF_OpenFont("Arial.ttf", 24);
    if (!font) {
        printf("Font Load Error: %s\n", TTF_GetError());
        return false;
    }

    /*
    soundFix = Mix_LoadWAV("fix.wav");
    soundLine = Mix_LoadWAV("line.wav");

    if (!soundFix || !soundLine) {
        printf("Sound Load Error: %s\n", Mix_GetError());
        return false;
    }
*/
    return true;
}


void drawText(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}


void drawGrid() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (grid[y][x] != 0) {
                drawBlock(x, y, grid[y][x] - 1);
            }
        }
    }
}

void drawPiece() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x] != 0) {
                drawBlock(currentPiecePosX + x, currentPiecePosY + y, currentPieceColor - 1);
            }
        }
    }
}

bool checkCollision(int x, int y, int shape[4][4]) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (shape[py][px] != 0) {
                int boardX = x + px;
                int boardY = y + py;
                if (boardX < 0 || boardX >= WIDTH || boardY >= HEIGHT || (boardY >= 0 && grid[boardY][boardX] != 0)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void rotatePiece() {
    int temp[4][4];

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[y][x] = currentPiece[3 - x][y];
        }
    }

    if (!checkCollision(currentPiecePosX, currentPiecePosY, temp)) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                currentPiece[y][x] = temp[y][x];
            }
        }
    }
}


void movePieceDown() {
    if (checkCollision(currentPiecePosX, currentPiecePosY + 1, currentPiece)) {
        fixPiece();
    } else {
        currentPiecePosY++;
    }
}

void movePieceLeft() {
    if (!checkCollision(currentPiecePosX - 1, currentPiecePosY, currentPiece)) {
        currentPiecePosX--;
    }
}

void movePieceRight() {
    if (!checkCollision(currentPiecePosX + 1, currentPiecePosY, currentPiece)) {
        currentPiecePosX++;
    }
}


void fixPiece() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece[y][x] != 0) {
                grid[currentPiecePosY + y][currentPiecePosX + x] = currentPieceColor;
            }
        }
    }

    clearLines();

    currentPieceColor = rand() % 7 + 1;
    currentPiecePosX = WIDTH / 2 - 2;
    currentPiecePosY = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentPiece[y][x] = shapes[currentPieceColor - 1][y][x];
        }
    }

    if (checkCollision(currentPiecePosX, currentPiecePosY, currentPiece)) {
        gameOver();
    }
}



void handleEvents(SDL_Event event) {
    static Uint32 lastKeyTime = 0;
    Uint32 now = SDL_GetTicks();

    if (event.type == SDL_KEYDOWN && now - lastKeyTime > 50) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                movePieceLeft();
            break;
            case SDLK_RIGHT:
                movePieceRight();
            break;
            case SDLK_DOWN:
                movePieceDown();
            break;
            case SDLK_UP:
                rotatePiece();
            break;
            case SDLK_ESCAPE:
                SDL_Quit();
                break;
            break;
        }
        lastKeyTime = now;
    }
}


void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawGrid();
    drawBorder();
    drawPiece();

    char buffer[64];
    SDL_Color white = {255, 255, 255, 255};
    sprintf(buffer, "Score: %d", score);
    drawText(buffer, 10, SCREEN_HEIGHT + 5, white);

    sprintf(buffer, "Level: %d", level);
    drawText(buffer, 200, SCREEN_HEIGHT + 5, white);

    SDL_RenderPresent(renderer);
}


