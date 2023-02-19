#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

int SIZE = 7;

void initializeGame(char **game) { // Créer le tablea de la partie
    for (int i = 0; i < SIZE; i++) {
        game[i] = malloc(sizeof(char) * SIZE); // Alloue de l'espace mémoire pour le tableau de jeu
        for (int j = 0; j < SIZE; j++) {
            game[i][j] = '+';
        }
    }
}
void unloadGame(char **game) { // Permet de free le tableau de la prtie afin de ne pas laisser de fuites mémoires
    for (int i = 0; i < SIZE; i++) { // Free le tableau
        free(game[i]);
    }
    free(game);
}
void displayGame(char **game) { // Affichage utilisé afin de vois le jeu en version textuel
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

void checkFull(char **game, int *win) { // Regarde si il reste des coups possible
    int full = 1;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (game[i][j] == '+') { // Regarde si il y a encore une case qui a pour valeur '+'
                full = 0; // Si il n'est pas plein la game continue
            }
        }
    }
    if (full) { // Si il ne reste pas de case, donc que 'full' est toujours égale à 1
        *win = 100; // Alors la partie est fini car aucun coup n'est possible
    }
}
void checkLine(char **game, char color, int line, int *win) { // Permet de voir si une des lignes contiens 4 jetons
    int sum;
    for (int i = 0; i < SIZE; i++) {
        if (game[i][line] == color) { //Vérifie si la couleur est 4 fois la même en incrémentant sum
            sum++;
        } else {
            sum = 0; // Sinon sum reviens à 0
        }
        if (sum == 4) { //si sum arrive juèsqu'a 4, donc qu'il y a un gagnant
            if (color == 'R') { // Si la couleur vérifier est R, accord victoire à R
                *win = 10;
            } else { // Sinon c'est l'adversaire qui remporte la victoire
                *win = 20;
            }
        }
    }
}
void checkColumn(char **game, char color, int column, int *win) { // Permet de voir si une des colonnes contiens 4 jetons
    int sum;
    for (int i = 0; i < SIZE; i++) {
        if (game[column][i] == color) { //Vérifie si la couleur est 4 fois la même en incrémentant sum
            sum++;
        } else { // Sinon sum reviens à 0
            sum = 0;
        }
        if (sum == 4) { //si sum arrive juèsqu'a 4, donc qu'il y a un gagnant
            if (color == 'R') { // Si la couleur vérifier est R, accord victoire à R
                *win = 10;
            } else { // Sinon c'est l'adversaire qui remporte la victoire
                *win = 20;
            }
        }
    }
}
void checkDiagonals(char **game, char color, int line, int column, int *win) { // Permet de voir si une des diagonales contiens 4 jetons
    int sum;

    sum = 0;
    for (int i = column - 3, j = line - 3; i <= column + 3 && j <= line + 3; i++, j++) {  // Prend en compte qu'il n'y a que 3 hauteur possible auxquels noous pouvosn faire une diagonale, on regarde les diagonales de droite vers la gauche
        if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) { // On compte les diagonales possbiles
            if (game[i][j] == color) { //Vérifie si la couleur est 4 fois la même en incrémentant sum
                sum++;
            } else { // Sinon sum reviens à 0
                sum = 0;
            }
            if (sum == 4) { //si sum arrive juèsqu'a 4, donc qu'il y a un gagnant
                if (color == 'R') { // Si la couleur vérifier est R, accord victoire à R
                    *win = 10;
                } else { // Sinon c'est l'adversaire qui remporte la victoire
                    *win = 20;
                }
            }
        }
    }

    sum = 0;
    for (int i = column + 3, j = line - 3; i >= column - 3 && j <= line + 3; i--, j++) { // Nous vérifions encore les diagonales mais celles ci sont de droite vers la gauche
        if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) { // On compte les diagonales possbiles
            if (game[i][j] == color) { //Vérifie si la couleur est 4 fois la même en incrémentant sum
                sum++;
            } else { // Sinon sum reviens à 0
                sum = 0;
            }
            if (sum == 4) { //si sum arrive juèsqu'a 4, donc qu'il y a un gagnant
                if (color == 'R') { // Si la couleur vérifier est R, accord victoire à R
                    *win = 10;
                } else { // Sinon c'est l'adversaire qui remporte la victoire
                    *win = 20;
                }
            }
        }
    }
}
void checkWin(char **game, char color, int column, int line, int *win) { // Appelle les différentes vérifiaction de victoires
    checkFull(game, win);
    checkLine(game, color, line, win);
    checkColumn(game, color, column, win);
    checkDiagonals(game, color, line, column, win);
}

int end(int winner, int loser,int even) { // Affiche si le jouer a gagner le résultat de la partié pour le joueur
    if(even){ // Si la fonction est even==1,
        send(winner, "EQUALITY", 8, 0); // envoie égalité au joueur 1
        send(loser, "EQUALITY", 8, 0); // envoie égalité au joueur 2
        return 1; // Fin de partie
    }
    printf("Le joueur %d a gagné !\n",winner);
    send(winner, "YOUWIN!!", 8, 0); // envoie le paquet YOUWIN!! au gagnant
    send(loser, "YOULOSE!", 8, 0); // envoie le paquet YOULOSE! au perdant
    return 1;
}

int putOnColumn(char **game, int column, char color, int*win) { // Action de jouer un coup dans le jeu
    if (column < 1 || column > 7) return 0; // Si le coup n'est pas possible, retourne 0
    int count = SIZE - 1;
    while (game[column - 1][count] != '+') { // Choisit l'endroit
        count--;
    }
    if (count < 0) return 0; // Si il n'y as plus de place, return 0
    game[column - 1][count] = color; // Place le jeton de la couleur
    printf("[DEBUG] Placed token on column %d, he fell to the lines %d.\n", column, count + 1);
    checkWin(game,color,column-1,count,win); // Vérfie si il y a un gagnant
    return 1; // Le coup a été joué, cela return 1
}

char getCharOf(int player){ // sélectionne la bonne couleur
    if(player%2==0){
        return 'R';
    } else {
        return 'B';
    }
}

void playerTurn(int playerID, int socketPlayer1, int socketPlayer2, char **game, int * win) { // Fait l'action entière d'un tour du jeu
    printf("C'est au tour du joueur %d !\n",playerID);
    send(socketPlayer1, "YOURTURN", 8, 0); // Envoie le paquet YOURTURN au joueur 1
    send(socketPlayer2, "WAITTURN", 8, 0); // Envoie le paquet WAITTURN au joueur 2
    int column;
    recv(socketPlayer1, &column, sizeof(column), 0); // Attend le coup du joueur 1
    fflush(stdout);
    printf("RECEIVED PLAYER TURN POS : column %d",column);
    if(putOnColumn(game,column,getCharOf(playerID),win)){ // Ajoute le jeton au dessus de la colonne
        printf("Envoi du coup à l'autre joueur ! \n");
        send(socketPlayer2, &column, sizeof(column), 0);// Envoie du coup au joueur adverse
    } else {
        *win = 999; // Si le coup n'est pas réussi, déclenche une erreur
    }
}


int connect4Server(int socketPlayer1, int socketPlayer2) { // Partie serveur du jeu permettant de lier les deux joueurs

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

    char **game = malloc(sizeof(char *) * SIZE); // Créer le tableau
    initializeGame(game); // Commence la partie
    printf("Voici le plateau de départ :\n");
    displayGame(game);


    int win = 0;
    int turn = 0;

    while (!win) {
        turn++;
        displayGame(game);

        if (turn % 2 == 0){ // tour du joueur 1
            playerTurn(1, socketPlayer1, socketPlayer2, game, &win);
        } else if (turn % 2 == 1){ // tour du joueur 2
            playerTurn(2, socketPlayer2, socketPlayer1, game, &win);
        }
    }

    if (win == 100) { // Vérifier les valeurs de victoire
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

    unloadGame(game); // En cas de vitoire, on libère la mémoire
    return 0;
}

