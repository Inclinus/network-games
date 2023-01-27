#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../sdl-utils/SDLUtils.h"

void checkFull(char **game, int *win);
void checkLine(char **game, char color, int line, int *win);
void checkColumn(char **game, char color, int column, int *win);
void checkDiagonals(char **game, char color, int line, int column, int *win);
void checkWin(char **game, char color, int column, int line, int *win);
void displayGame(char **game);
int putOnColumn(char **game, int line, char color, int *win);
void initializeGame(char **game);

int SIZE = 7;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

char red = 'R';
char blue = 'B';

// SDL Bool to do the game loop
SDL_bool program_launched;

int main() {


    char **game = malloc(sizeof(char *) * SIZE);

    initializeGame(game);
    printf("Voici le plateau de départ :\n");
    displayGame(game);

    // win values :
    // 100 = null : board is full
    // 10 = RED won
    // 20 = BLUE won
    int win = 0;
    int turn = 0;
    while (!win) {
        int columnChosen = 0;
        int noError = 0;
        if (turn % 2 == 0) {
            while (columnChosen < 1 || columnChosen > 7) {
                printf("[CONNECT 4] BLUE turn\n");
                printf("[CONNECT 4] Chose a column by her number :\n");
                scanf("%d", &columnChosen);
                noError = putOnColumn(game, columnChosen, blue, &win);
                if (!noError) {
                    displayGame(game);
                    printf("[CONNECT 4] Your choice is out of the game.\n");
                    columnChosen = 0;
                }
            }
        } else {
            while (columnChosen < 1 || columnChosen > 7) {
                printf("[CONNECT 4] RED turn\n");
                printf("[CONNECT 4] Chose a column by her number :\n");
                scanf("%d", &columnChosen);
                noError = putOnColumn(game, columnChosen, red, &win);
                if (!noError) {
                    displayGame(game);
                    printf("[CONNECT 4] Your choice is out of the game.\n");
                    columnChosen = 0;
                }
            }
        }
        turn++;
    }

    if (win == 100) {
        printf("[CONNECT 4] Nobody has won ! You filled the board without making any lines of 4 of your tokens.\n[CONNECT 4] See you soon !");
    } else if (win == 10) {
        printf("[CONNECT 4] RED has won !\n[CONNECT 4] See you soon !");
    } else if (win == 20) {
        printf("[CONNECT 4] BLUE has won !\n[CONNECT 4] See you soon !");
    }

    return 0;
}


void checkFull(char **game, int *win) {
    int full = 1;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game[i][j] == '+') {
                full = 0;
            }
        }
    }
    if (full) {
        *win = 100;
    }
}

void checkLine(char **game, char color, int line, int *win) {
    int sum;
    for (int i = 0; i < SIZE; i++) {
        if (game[i][line] == color) {
            sum++;
        } else {
            sum = 0;
        }
        if (sum == 4) {
            if (color == 'R') {
                *win = 10;
            } else {
                *win = 20;
            }
        }
    }
}

void checkColumn(char **game, char color, int column, int *win) {
    int sum;
    for (int i = 0; i < SIZE; i++) {
        if (game[column][i] == color) {
            sum++;
        } else {
            sum = 0;
        }
        if (sum == 4) {
            if (color == 'R') {
                *win = 10;
            } else {
                *win = 20;
            }
        }
    }
}

void checkDiagonals(char **game, char color, int line, int column, int *win) {
    int sum;

    sum = 0;
    for (int i = column - 3, j = line - 3; i <= column + 3 && j <= line + 3; i++, j++) {
        if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) {
            if (game[i][j] == color) {
                sum++;
            } else {
                sum = 0;
            }
            if (sum == 4) {
                if (color == 'R') {
                    *win = 10;
                } else {
                    *win = 20;
                }
            }
        }
    }

    sum = 0;
    for (int i = column + 3, j = line - 3; i >= column - 3 && j <= line + 3; i--, j++) {
        if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) {
            if (game[i][j] == color) {
                sum++;
            } else {
                sum = 0;
            }
            if (sum == 4) {
                if (color == 'R') {
                    *win = 10;
                } else {
                    *win = 20;
                }
            }
        }
    }
}

void checkWin(char **game, char color, int column, int line, int *win) {
    checkFull(game, win);
    checkLine(game, color, line, win);
    checkColumn(game, color, column, win);
    checkDiagonals(game, color, line, column, win);
}


void displayGame(char **game) {
    printf("\n-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        printf("| %d ", i + 1);
    }
    printf("|\n");
    printf("-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("| %c ", game[j][i]);
        }
        printf("|\n");
    }
    printf("-----------------------------\n");
}

int putOnColumn(char **game, int line, char color, int *win) {
    if (line < 1 || line > 7) return 0;
    int count = SIZE - 1;
    while (game[line - 1][count] != '+') {
        count--;
    }
    if (count < 0) return 0;
    game[line - 1][count] = color;
    displayGame(game);
    printf("[DEBUG] You placed your token on column %d, he fell to the line %d.\n", line, count + 1);
    checkWin(game, color, line - 1, count, win);
    return 1;
}


void initializeGame(char **game) {
    for (int i = 0; i < SIZE; i++) {
        game[i] = malloc(sizeof(char) * SIZE);
        for (int j = 0; j < SIZE; j++) {
            game[i][j] = '+';
        }
    }
}
