#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "tictactoe/tictactoe.h"
#include "../events/EventManager.h"
#include "connect4/connect4.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include "sdl-client.h"

// action = 1 -> login
// action = 2 -> register
int login(int * socketClient, int action){
    if (action == 1) {
        send(*socketClient, "LOGINCLI", 8, 0);
    } else if (action == 2) {
        send(*socketClient, "REGISTER", 8, 0);
    }
    char buffer[25];
    printf("Quel est votre pseudo : \n");
    scanf("%s", buffer);
    send(*socketClient, buffer, sizeof(buffer), 0);
    printf("Quel est votre mot de passe : \n");
    scanf("%s", buffer);
    send(*socketClient, buffer, sizeof(buffer), 0);
    char result[3];
    recv(*socketClient, result, sizeof(result), 0);
    result[2] = '\0';
    printf("RECU : %s\n", result);
    if(strcmp(result, "OK") == 0){
        printf("OK !\n");
        return 1;
    }else{
        printf("NOK !\n");
        return 0;
    }
}

int main() {
    eventManagerInit();
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // gethostbyname() -> permet d'utilise le DNS
    // projetc.neo-serv.fr -> 92.222.131.57

    struct hostent *ipserveur;
    //ipserveur = gethostbyname("localhost");
    ipserveur = gethostbyname("projetc.neo-serv.fr");

    if (ipserveur == NULL) {
        printf("ERREUR, l'host n'a pas été trouver\n");
        exit(0);
    }

    //addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)*ipserveur->h_addr_list));
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    if (connect(socketClient, (const struct sockaddr *) &addrServer, sizeof(addrServer)) < 0) {
        perror("ERREUR DE CONNEXION");
        exit(1);
    }
    printf("[DEBUG] CONNECTER !\n");

    send(socketClient, "LOGIN", 5, 0);

    int action_login;
    printf("1 - Se connecter\n 2 - S'inscrire\n");
    scanf("%d", &action_login);
    if (action_login == 1) {
        while (login(&socketClient, 1) == 0);
    } else if (action_login == 2) {
        login(&socketClient, 2);
    } else {
        printf("ERREUR DE CHOIX !\n");
        exit(1);
    }
    // Clear event queues to be sure no events is stucked
    clearQueues();
    
    // TODO implement choice of game here
    //   Utilise le pollevent sdl (comme sur tictactoe) pour détecter un clic de souris
    //   COmpare la position du clic avec tes boutons et lance en fonction tictactoe ou connect4
    //   PS : pour l'instant connect4 est une fenêtre vide avec un texte connect 4

    printf("[DEBUG] FIN DE L'AUTHENTIFICATION !\n");
    
    SDL_Renderer * rendererMenu = NULL;
    SDL_Window * windowMenu = NULL;

    windowMenu = SDL_CreateWindow("MORPION",50,50,720,480,0);
    rendererMenu = SDL_CreateRenderer(windowMenu,-1,0);

    int flag = 0;
    int outputMenu;
    
    while (!flag)
    {
        outputMenu = MainMenu(rendererMenu);
        switch (outputMenu)
        {
        case 1:
            printf("CASE 1 \n");
            send(socketClient, "QUEUE", 5, 0);
            char buffer[12];
            recv(socketClient, buffer, sizeof(buffer), 0);
            buffer[11] = '\0';
            printf("RECU : %s", buffer);

            if(strcmp(buffer, "STARTLOBBYH") == 0){
                printf("Le lobby a été crée et tu es l'hote !\n");
                outputMenu = Game(rendererMenu);
                switch (outputMenu)
                {
                    case 1:
                        if(strcmp(buffer, "STARTLOBBYH") == 0){
                            send(socketClient, "TICTACTOE", 9, 0);
                        }
                        char bufferstart1[10];
                        recv(socketClient, bufferstart1, sizeof(bufferstart1), 0);
                        bufferstart1[9] = '\0';
                        if(strcmp(bufferstart1, "TICTACTOE") == 0){
                            printf("Le jeu va commencer !\n");
                            tictactoe(&socketClient);
                        } else if (strcmp(bufferstart1, "NCONNECT4") == 0){
                            printf("Le jeu va commencer !\n");
                            connect4(&socketClient);
                        } else {
                            printf("ERREUR DE RECEPTION !\n");
                        }
                        //launch Morpion
                        break;
                    case 2:
                        if(strcmp(buffer, "STARTLOBBYH") == 0){
                            send(socketClient, "NCONNECT4", 9, 0);
                        }
                        char bufferstart2[10];
                        recv(socketClient, bufferstart2, sizeof(bufferstart2), 0);
                        bufferstart2[9] = '\0';
                        if(strcmp(bufferstart2, "TICTACTOE") == 0){
                            printf("Le jeu va commencer !\n");
                            tictactoe(&socketClient);
                        } else if (strcmp(bufferstart2, "NCONNECT4") == 0){
                            printf("Le jeu va commencer !\n");
                            connect4(&socketClient);
                        } else {
                            printf("ERREUR DE RECEPTION !\n");
                        }
                        //launch Puissance4
                        break;
                    case 3:
                        break;
                }
            } else if (strcmp(buffer, "STARTLOBBYJ") == 0){
                printf("Le lobby a été crée et tu es un joueur !\n");
                char bufferstart[10];
                recv(socketClient, bufferstart, sizeof(bufferstart), 0);
                bufferstart[9] = '\0';
                if(strcmp(bufferstart, "TICTACTOE") == 0){
                    printf("Le jeu va commencer !\n");
                    tictactoe(&socketClient);
                } else if (strcmp(bufferstart, "NCONNECT4") == 0){
                    printf("Le jeu va commencer !\n");
                    connect4(&socketClient);
                } else {
                    printf("ERREUR DE RECEPTION !\n");
                }
            } else {
                printf("ERREUR DE RECEPTION !\n");
            }
            break;
        case 2:
            // launch stat
            break;
        case 3:
            SDL_DestroyWindow(windowMenu);
            SDL_Quit();
            flag =1;
            break;

        }
    }

    //char buffer[25];
    //recv(socketClient, buffer, sizeof(buffer), 0);

    //tictactoe(&socketClient);

    close(socketClient);

    return 0;
}
