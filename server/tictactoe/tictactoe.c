#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

struct Player {
    char *nickname;
    int id;
};

void processPlayerTurn(int playerID, int socketPlayer1, int socketPlayer2, int **board, int player, int * flag);

int win(int winner, int loser, int player);
int draw(int player1, int player2);

void printboard(int ** board) // Permet d'afficher le jeu en version textuel
{
    for (int i = 2; i >=0; --i) // Premier tableau
    {
        for (int y = 0; y < 3; ++y) // Second tableau
        {
            printf("| %d ",board[i][y]); // affichage de la case
        }
        printf("| \n");
    }
}

int turn(int ** board, int player, int px, int py) // Permet de vérifier si un coup est possible
{
    if(player%2==0 && board[px][py]!=1 && board[px][py]!=2){ // Si le coup est du joueur 0, et que la case n'est pas remplise par un autre joueur
        board[px][py]=1; // Case prise par le joueur 0
        return 1; // Le coup s'est bien déroulé
    }
    else if(player%2==1 && board[px][py]!=1 && board[px][py]!=2){ // Si le coup est du joueur 0, et que la case n'est pas remplise par un autre joueur
        board[px][py]=2; // Case prise par le joueur 1
        return 1; // Le coup s'est bien déroulé
    }else{
        printf("Coup Impossible ! Une erreur est survenue\n"); // Affichage d'erruer survenu pendant le coup
        printboard(board); // Affichage du terrain
        return 0; // le coup a eu un problème
    }
}
int winCondition(int ** board,int verifNumber){ // Vérifie si un joueur a gagner la partie

    for (int i = 0; i < 2; i++) { // Boucle sur toutes les colonnes
        if (board[i][0] == verifNumber && board[i][1] == verifNumber && board[i][2] == verifNumber) { // Vérifications des colonnes
            return 1; // Il y a un gagnant
        }
    }

    for (int y = 0; y < 2; y++) { // Boucle sur toutes les lignes
        if (board[0][y] == verifNumber && board[1][y] == verifNumber && board[2][y] == verifNumber) { // Vérifications des lignes
            return 1; // Il y a un gagnant
        }
    }

    if (board[0][0] == verifNumber && board[1][1] == verifNumber && board[2][2] == verifNumber) { // vérification de la première diagonale
        return 1; // Il y a un gagnant
    }
    if (board[0][2] == verifNumber && board[1][1] == verifNumber && board[2][0] == verifNumber) { // vérification de la première diagonale
        return 1; // Il y a un gagnant
    }

    return 0; // le n'y a pas de gagnant
}

int tictactoe(int socketPlayer1, int socketPlayer2) { // fonction principale permettant de lancer le jeu

    int hostCount = 0;
    int playerCount = 0;
    // On attend que les deux joueurs soient prêts
    while (hostCount != 2 || playerCount != 1) {
        printf("SEND PING !!!\n");
        fflush(stdout);

        if (hostCount == 0) {
            char buffer1[5];
            char buffer2[5];
            send(socketPlayer1, "PING", 4, 0);
            send(socketPlayer1, "PING", 4, 0);

            recv(socketPlayer1, buffer1, 4, 0);
            buffer1[4] = '\0';
            printf("[1] buffer1 : %s\n", buffer1);
            if (strcmp(buffer1, "DEAD") == 0) {
                hostCount++;
            }

            recv(socketPlayer1, buffer2, 4, 0);
            buffer2[4] = '\0';
            printf("[2] buffer2 : %s\n", buffer2);
            if (strcmp(buffer2, "DEAD") == 0) {
                hostCount++;
            }
        } else if (hostCount == 1) {
            char buffer[5];
            send(socketPlayer1, "PING", 4, 0);

            recv(socketPlayer1, buffer, 4, 0);
            buffer[4] = '\0';
            printf("[3] buffer1 : %s\n", buffer);
            if (strcmp(buffer, "DEAD") == 0) {
                hostCount++;
            }
        }

        if (playerCount != 1) {
            char buffer[5];
            send(socketPlayer2, "PING", 4, 0);
            recv(socketPlayer2, buffer, 4, 0);
            buffer[4] = '\0';
            printf("[4] buffer : %s\n", buffer);
            if (strcmp(buffer, "DEAD") == 0) {
                playerCount++;
            }
        }

    }

    printf("PARTIE COMMENCE !\n");
    send(socketPlayer1, "START", 5, 0);
    send(socketPlayer2, "START", 5, 0);

    int ** board;
    int * row;
    board=malloc(sizeof(int*)*3); // Créer la 1ère dimension du tableau

    for (int i = 0; i < 3; ++i) // boucle sur le tableau de jeu
    {
        row=malloc(sizeof(int)*3); // Ajoute la 2ème dimensions au tableau de jeu
        board[i]=row;

    }
    for (int i = 0; i < 3; ++i) // Premier affichage du tableau de jeu
    {
        for (int y = 0; y < 3; ++y)
        {

            board[i][y]=0; // remplis le tableau de jeu avec des 0 en faisant le premier affichage
            printf("| %d ",board[i][y]);
        }
        printf("| \n");
    }


    int player=1;
    int flag=0;

    while (flag==0) { // tant que des coups sont jouables et qu'ils n'y as pas de gagnant
        printboard(board);
        player+=1;

        if (player%2==0){ // Vérifie si c'est le tour du joueur 0
            processPlayerTurn(1,socketPlayer1, socketPlayer2, board, player,&flag);
        } else if (player%2==1){ // Vérifie si c'est le tour du joueur 1
            processPlayerTurn(2,socketPlayer2, socketPlayer1, board, player,&flag);
        }

        if (winCondition(board,1)){ // Vérifie toutes les conditions de victoire pour le joueurs 1
            flag = win(socketPlayer1, socketPlayer2, 1); // Retourne 1 en cas de victoire
        } else if (winCondition(board,2)){// Vérifie toutes les conditions de victoire pour le joueurs 2
            flag = win(socketPlayer2, socketPlayer1, 2); // Retourne 1 en cas de victoire
        }

        if(player>=10){ // au 10ème coup la partie s'arrête car le tableau ne peux accueillir que 9 coup
            flag = draw(socketPlayer2,socketPlayer1);
        }
    }
    free(row); // Free le la 2ème dimension du tableau
    free(board); // Free le la 1ère dimension du tableau

    return 0;
}

int win(int winner, int loser, int player) { // Transmet les résultats de la partie aux utilisateurs en cas de victoire d'un des joueurs
    printf("Le joueur %d a gagné !\n",player);
    send(winner, "YOUWIN!!", 8, 0);
    send(loser, "YOULOSE!", 8, 0);
    return 1;
}

int draw(int player1, int player2){ // Transmet les résultats de la partie aux utilisateurs en cas d'égalité
    send(player1, "DRAWDRAW", 8, 0);
    send(player2, "DRAWDRAW", 8, 0);
    return 1;
}

void processPlayerTurn(int playerID,int socketPlayer1, int socketPlayer2, int **board, int player, int * flag) { // Fonction permettant de jouer un tour
    printf("C'est au tour du joueur %d !\n",playerID);
    send(socketPlayer1, "YOURTURN", 8, 0); // Envoie de l'instruction YOURTURN au joueur qui doit jouer
    send(socketPlayer2, "WAITTURN", 8, 0); // Envoie de l'instruction WAITTURN au joueur qui doit jouer
    int px;
    int py;
    recv(socketPlayer1, &px, sizeof(px), 0); // Attente des coordonées X du coup du joueur
    recv(socketPlayer1, &py, sizeof(py), 0); // Attente des coordonées Y du coup du joueur
    if(turn(board,player,px,py)){
        printf("Envoi du coup à l'autre joueur ! \n");
        send(socketPlayer2, &px, sizeof(px), 0); // Envoie des coordonnées X du coup venant jouer au joueurs ayant recu l'instruciton WAITTURN
        send(socketPlayer2, &py, sizeof(py), 0); // Envoie des coordonnées Y du coup venant jouer au joueurs ayant recu l'instruciton WAITTURN
    } else {
        *flag = 1;
    }
}
