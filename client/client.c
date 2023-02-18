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
#include "../sdl-utils/SDLUtils.h"
#include "guis/MainMenu.h"
#include <SDL2/SDL_ttf.h>
#include <pthread.h>

SDL_bool clientRunning = SDL_TRUE;
int * clientSocket;

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
    clientSocket = &socketClient;



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

    printf("[DEBUG] FIN DE L'AUTHENTIFICATION !\n");

    initSDLGUIs(&clientRunning,&socketClient);

    while (clientRunning) {
        loadMainMenu();
    }

    close(socketClient);

    return 0;
}