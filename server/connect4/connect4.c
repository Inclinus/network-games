#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

int SIZE = 7;

void initializeGame(char **game) {
    for (int i = 0; i < SIZE; i++) {
        game[i] = malloc(sizeof(char) * SIZE);
        for (int j = 0; j < SIZE; j++) {
            game[i][j] = '+';
        }
    }
}
void unloadGame(char **game) {
    for (int i = 0; i < SIZE; i++) {
        free(game[i]);
    }
    free(game);
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

int end(int winner, int loser,int even) {
    if(even){
        send(winner, "EQUALITY", 8, 0);
        send(loser, "EQUALITY", 8, 0);
        return 1;
    }
    printf("Le joueur %d a gagné !\n",winner);
    send(winner, "YOUWIN!!", 8, 0);
    send(loser, "YOULOSE!", 8, 0);
    return 1;
}

int putOnColumn(char **game, int column, char color, int*win) {
    if (column < 1 || column > 7) return 0;
    int count = SIZE - 1;
    while (game[column - 1][count] != '+') {
        count--;
    }
    if (count < 0) return 0;
    game[column - 1][count] = color;
    printf("[DEBUG] Placed token on column %d, he fell to the lines %d.\n", column, count + 1);
    checkWin(game,color,column-1,count,win);
    return 1;
}

char getCharOf(int player){
    if(player%2==0){
        return 'R';
    } else {
        return 'B';
    }
}

void playerTurn(int playerID, int socketPlayer1, int socketPlayer2, char **game, int * win) {
    printf("C'est au tour du joueur %d !\n",playerID);
    send(socketPlayer1, "YOURTURN", 8, 0);
    send(socketPlayer2, "WAITTURN", 8, 0);
    int column;
    recv(socketPlayer1, &column, sizeof(column), 0);
    fflush(stdout);
    printf("RECEIVED PLAYER TURN POS : column %d",column);
    if(putOnColumn(game,column,getCharOf(playerID),win)){
        printf("Envoi du coup à l'autre joueur ! \n");
        send(socketPlayer2, &column, sizeof(column), 0);
    } else {
        *win = 999;
    }
}


int connect4Server(int socketPlayer1, int socketPlayer2) {

    char buffer1[5];
    char buffer2[5];
    while (strcmp(buffer1, "PONG") != 0 && strcmp(buffer2, "PONG") != 0) {
        printf("SEND PING !!!\n");
        fflush(stdout);
        send(socketPlayer1, "PING", 4, 0);
        send(socketPlayer2, "PING", 4, 0);
        recv(socketPlayer1, buffer1, 4, 0);
        buffer1[4] = '\0';
        printf("buffer1 : %s\n", buffer1);
        recv(socketPlayer2, buffer2, 4, 0);
        buffer2[4] = '\0';
        printf("buffer2 : %s\n", buffer2);
    }

    printf("PARTIE COMMENCE !\n");

    char **game = malloc(sizeof(char *) * SIZE);
    initializeGame(game);
    printf("Voici le plateau de départ :\n");
    displayGame(game);


    int win = 0;
    int turn = 0;

    while (!win) {
        turn++;
        displayGame(game);

        if (turn % 2 == 0){
            playerTurn(1, socketPlayer1, socketPlayer2, game, &win);
        } else if (turn % 2 == 1){
            playerTurn(2, socketPlayer2, socketPlayer1, game, &win);
        }
    }

    if (win == 100) {
        printf("[CONNECT 4] Nobody has won ! You filled the board without making any lines of 4 of your tokens.\n[CONNECT 4] See you soon !");
        end(socketPlayer1,socketPlayer2,1);
    } else if (win == 10) {
        printf("[CONNECT 4] RED has won !\n[CONNECT 4] See you soon !");
        end(socketPlayer2,socketPlayer1,0);
    } else if (win == 20) {
        printf("[CONNECT 4] BLUE has won !\n[CONNECT 4] See you soon !");
        end(socketPlayer1,socketPlayer2,0);
    } else if(win == 999){
        printf("[CONNECT 4] ERROR !\n[CONNECT 4] Something went wrong !");
    }

    unloadGame(game);
    return 0;
}

