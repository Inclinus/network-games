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
#include <SDL2/SDL.h>
#include "../sdl-utils/SDLUtils.h"

void displayConfig(SDL_Renderer * renderer, char * text){
    changeColor(renderer,255,255,255);
    createFilledRectangle(0,0,500,400,renderer);
    changeColor(renderer,0,0,0);
    createFilledRectangle(100,150,300,50,renderer);

    createTextZone(renderer,text,105,160,255,255,255);
    updateRenderer(renderer);
}

void configServer(){
    initSDL();

    SDL_Window * configWindow = SDL_CreateWindow("PARAMETRES",50,50,500,400,0);
    SDL_Renderer * configRenderer = SDL_CreateRenderer(configWindow, -1, 0);



    char * text = malloc(sizeof(char)*2);
    text[0] = ' ';
    text[1] = '\0';

    displayConfig(configRenderer,text);


    int quit = 0;
    while(!quit){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_QUIT:
                    quit=1;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_KP_ENTER){
                        quit=2;
                        break;
                    } else if(event.key.keysym.sym == SDLK_BACKSPACE){
                        int length = strlen(text);
                        if(length>0){
                            text = realloc(text,length*sizeof(char));
                            text[length-1] = '\0';
                        }
                    } else {
                        int length = strlen(text);
                        text = realloc(text,length* sizeof(char)+2);
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = keyname[0];

                        text[length] = keyChar;
                        text[length+1] = '\0';
                    }
            }
            displayConfig(configRenderer,text);
        }
    }


}


int main() {
    eventManagerInit();
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // gethostbyname() -> permet d'utilise le DNS
    // projetc.neo-serv.fr -> 92.222.131.57

    struct hostent *ipserveur;
    ipserveur = gethostbyname("localhost");
    //ipserveur = gethostbyname("projetc.neo-serv.fr");

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

    printf("En attente d'adversaire ...\n");

    char data[8];
    recv(socketClient, data, 8, 0);
    printf("[DEBUG] RECU : %s\n", data);
    if (strcmp(data, "NICKNAME")) {
        printf("Quel est votre pseudo : \n");
        scanf("%s", data);
        send(socketClient, data, sizeof(data), 0);
    } else {
        printf("ERREUR DE PROTOCOLE !\n");
    }
    // Clear event queues to be sure no events is stucked
    clearQueues();
    
    // TODO implement choice of game here
    //   Utilise le pollevent sdl (comme sur tictactoe) pour détecter un clic de souris
    //   COmpare la position du clic avec tes boutons et lance en fonction tictactoe ou connect4
    //   PS : pour l'instant connect4 est une fenêtre vide avec un texte connect 4


    configServer();
    //tictactoe(&socketClient);
    //connect4(&socketClient);


    close(socketClient);

    return 0;
}
