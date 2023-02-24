#include <sys/socket.h>
#include <pthread.h>
#include "ChoseGameMenu.h"
#include "../tictactoe/tictactoe.h"
#include "../connect4/connect4.h"
#include "MainMenu.h"
#include "../../events/EventManager.h"

SDL_bool * choseGameRunning = NULL;
int * choseGameClientSocket = NULL;
SDL_Renderer * choseGameRenderer = NULL;


void * choseGameNetworkListen();
void * choseGameSdlListen();

int choseGameMenu(SDL_Renderer * rendererMenu, int * socketClient){ // Fonction permettant de choisir le jeu à jouer
    clearQueues(); // Clear les queue
    choseGameRunning = malloc(sizeof(SDL_bool)); // Alloue la mémoire pour le booléen
    if(choseGameRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CHOSEGAMERUNNING SDLBOOL");
    }
    *choseGameRunning = SDL_TRUE; // Set le booléen a SDL_TRUE

    choseGameClientSocket = socketClient; // Set le socket client
    choseGameRenderer = rendererMenu; // Set le renderer

    pthread_t sdl_thread; // Création du listener SDL
    pthread_create(&sdl_thread, NULL, choseGameSdlListen, NULL);

    pthread_t network_thread; // Création du listener NETWORK
    pthread_create(&network_thread, NULL, (void*)choseGameNetworkListen, socketClient);


    while(*choseGameRunning){ // Tant que le menu est en train de tourner
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) { // Ecoute tout les évènement
            switch (event->type) {
                case SDL: // Si l'évenement est de type SDL
                    if(strcmp(event->instructions, "LEAVE") == 0){ // Si l'instruction est LEAVE
                        send(*choseGameClientSocket, "LEAVEGAME", 9, 0); // Envoie une déconnexion au serveur
                        *choseGameRunning = SDL_FALSE; // Fin de la boucle
                        loadMainMenu(); // Renvoie le menu principale
                    } else if(strcmp(event->instructions, "TICTACTOE") == 0){
                        send(*choseGameClientSocket, "TICTACTOE", 9, 0);
                        *choseGameRunning = SDL_FALSE;
                        tictactoe(choseGameClientSocket,rendererMenu);
                    } else if(strcmp(event->instructions, "CONNECT4") == 0){
                        send(*choseGameClientSocket, "NCONNECT4", 9, 0);
                        *choseGameRunning = SDL_FALSE;
                        connect4(choseGameClientSocket,rendererMenu);
                    }
                    break;
                case NETWORK: // Si l'évenement est de type NETWORK
                    if(strcmp(event->instructions, "DISCONNECTED") == 0){ // Si l'instruction est DISCONNECTED
                        SDL_ExitWithError("DISCONNECTED FROM SERVER"); // Déconnecte le client du server
                    } else if(strcmp(event->instructions, "GAMEBREAK") == 0){ // Si l'instruction est GAMEBREAK
                        *choseGameRunning = SDL_FALSE; // Fin du menu choix de jeu
                        loadMainMenu(); // Renvoie sur le menu principale
                    }
                    break;
                default:
                    break;
            }
            free(event);
        }
    }
}


void * choseGameSdlListen(){

    Button goBack; // Définition des coordonnées du boutton goBack
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

    Button tictactoeButton; // Définition des coordonnées du boutton tictactoeButton
    tictactoeButton.beginX = 5;
    tictactoeButton.beginY = 100;
    tictactoeButton.endX = 345;
    tictactoeButton.endY = 410;

    Button connect4Button; // Définition des coordonnées du boutton connect4Button
    connect4Button.beginX = 375;
    connect4Button.beginY = 100;
    connect4Button.endX = 720;
    connect4Button.endY = 410;

    SDL_RenderClear(choseGameRenderer); // Clear le rendu SDL

    createTextZoneCentered(choseGameRenderer, "Quel jeu choisir", 720/2 ,50,255, 255, 255,48); // Titre SDL de la page

    createButton(choseGameRenderer,tictactoeButton, "Morpion"); // Crée le boutton Morpion
    createButton(choseGameRenderer,connect4Button, "Puissance 4"); // Crée le boutton Puissance 4
    createButton(choseGameRenderer,goBack, "retour"); // Crée le boutton retour

    updateRenderer(choseGameRenderer); // Met à jour le rendu

    while(*choseGameRunning){ // tant que le menu est celui du choix de jeu
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT: // si l'evnement est SDL_QUIT
                    sendEvent(createEvent(SDL,"LEAVE")); // Envoie au serveur la fin du choix de jeu
                    break;
                case SDL_MOUSEBUTTONDOWN: // Si  il appuie sur un boutton de la souris
                    ;
                    int x = event.button.x; // récupère les coordonnées de X
                    int y = event.button.y; // récupère les coordonnées de Y
                    if(x>tictactoeButton.beginX && x<tictactoeButton.endX && y<tictactoeButton.endY && y>tictactoeButton.beginY){ // Si les coordonnées sont dans le boutton tictactoe
                        sendEvent(createEvent(SDL,"TICTACTOE")); // envoie l'évènement TICTACTOE
                    }
                    if(x>connect4Button.beginX && x<connect4Button.endX && y<connect4Button.endY && y>connect4Button.beginY){ // Si les coordonnées sont dans le boutton CONNECT4
                        sendEvent(createEvent(SDL,"CONNECT4")); // envoie l'évènement CONNECT4
                    }
                    if(x>goBack.beginX && x<goBack.endX && y<goBack.endY && y>goBack.beginY){ // Si les coordonnées sont dans le boutton CONNECT4
                        sendEvent(createEvent(SDL,"LEAVE")); // envoie l'évènement LEAVE
                    }
                    break;
                default:
                    break;
            }
        }
    }
    pthread_exit(NULL);
}


void * choseGameNetworkListen() {
    NG_Event *disconnectEvent = createEvent(NETWORK,"DISCONNECTED");
    SDL_Log("LAUNCHING NETWORK THREAD CHOSEGAME");
    while(*choseGameRunning){
        char data[10];
        memset(data, '\0', sizeof(data));
        if (recv(*choseGameClientSocket, data, sizeof(data), 0) <= 0) { // Si le paquet est vide, se déconnecte
            sendEvent(disconnectEvent); // Envoie l'évènement de déconnexion
            break;
        } else if (strstr("PING", data) != NULL || strcmp("PINGPING",data) == 0 || strcmp("PING",data) == 0) { // Si l'évenement est égale à PING
            SDL_Log("[CHOSEGAME NETWORK LISTENER] PING RECEIVED");
            break;
        } else {
            NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));// Allocation mémoire receivedDataEvent
            if(receivedDataEvent==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT");
            }
            receivedDataEvent->type = NETWORK; // receivedDataEvent type NETWORK
            unsigned long len = strlen(data); // Prend la longueur de sata
            SDL_Log("[CHOSEGAME NETWORK LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
            receivedDataEvent->instructions = malloc(sizeof(char)*len); // Instruction allocation mémoire
            if(receivedDataEvent->instructions==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT INSTRUCTIONS");
            }
            strcpy(receivedDataEvent->instructions,data); // Met data dans instruction
            sendEvent(receivedDataEvent); // Envoie le paquet crée
        }
    }
    send(*choseGameClientSocket, "DEAD", 4, 0);
    SDL_Log("EXITING NETWORK THREAD CHOSEGAMEMENU");
    pthread_exit(NULL);
}

