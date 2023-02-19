#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <pthread.h>
#include <regex.h>
#include <unistd.h>
#include "../../sdl-utils/SDLUtils.h"
#include "connect4.h"
#include "../../events/EventManager.h"

void checkFull(char **game, int *win);
void checkLine(char **game, char color, int line, int *win);
void checkColumn(char **game, char color, int column, int *win);
void checkDiagonals(char **game, char color, int line, int column, int *win);
void checkWin(char **game, char color, int column, int line, int *win);

SDL_bool tryPlay(char **game, int column, char color);

void initializeGame(char **game);
void displayGameBoard(char **game);

void *sdlListener();
void *networkListener();

void debugConnectBoard(char **game);
int calculateBoardColumnPos(Sint32 x);

void setDisplayedInfo(char * text);
void setDisplayedFeedback(char * text);


int SIZE = 7;

SDL_Window *windowConnect4 = NULL;
SDL_Renderer *rendererConnect4 = NULL;

int WINDOW_WIDTH = 700; // Defini la Longeur
int WINDOW_HEIGHT = 800; // Defini Largeur

char RED = 'R';
char BLUE = 'B';

// SDL Bool to do the game loop
SDL_bool connect_launched = SDL_TRUE;

SDL_bool connect4QuitForcedByPlayer = SDL_FALSE;

char * actualDisplayedInfo;
char * actualDisplayedFeedback;

int * connect4ClientSocket;

int connect4(int * socketClient) {
    clearQueues();
    setDisplayedInfo("Bienvenue !");
    connect4ClientSocket = socketClient;
    initSDL();
    windowConnect4 = SDL_CreateWindow("CONNECT 4", 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, 0); // Crée une fenêtre SDL
    rendererConnect4 = SDL_CreateRenderer(windowConnect4, -1, 0); // Crée un rendu SDL

    pthread_t network_listener; // créer un thread pour le network_listener
    pthread_create(&network_listener, NULL, networkListener, NULL);
    pthread_t sdl_listener;// créer un thread pour le sdl_listener
    pthread_create(&sdl_listener, NULL, sdlListener, NULL);

    char **game = malloc(sizeof(char *) * SIZE); // allocation mémoire du tableau de jeu
    if(game==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CHAR ** GAME");
    }

    initializeGame(game); // commence la partie
    SDL_Log("Voici le plateau de départ :\n");
    displayGameBoard(game);

    regex_t posRegex;
    if (regcomp(&posRegex, "^POS[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        SDL_Log("Error: Could not compile regular expression\n");
        return 1;
    }
    regex_t enemyPosRegex;
    if (regcomp(&enemyPosRegex, "^ENEMY[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        SDL_Log("Error: Could not compile regular expression\n");
        return 1;
    }

    SDL_bool * yourTurn = malloc(sizeof(SDL_bool));  // Booléen determinant le tour
    if(yourTurn==NULL){
        SDL_ExitWithError("ERROR ALLOCATING YOURTURN BOOL");
    }
    *yourTurn = SDL_FALSE;

    while (connect_launched) { // tant que le client est connecté
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) { // tant qu'aucun évenement n'est reçu
            SDL_Log("CONNECT4 : EVENT RECEIVED");
            if (event->type == SDL) { // si l'évenement reçu est de type SDL
                SDL_Log("CONNECT4 : SDL EVENT RECEIVED %s",event->instructions);
                if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) { // Regarde si le pquet reçu correspond bien à un event connu
                    if(*yourTurn){ // si c'est votre tour
                        int column;
                        sscanf(event->instructions, "POS%d", &column); // Format l'entrée pour un évenement de position d'un jeton
                        if(tryPlay(game,column,BLUE)){ // Essaie de jouer un coup
                            debugConnectBoard(game);
                            SDL_Log("Coup Possible ! \n");
                            send(*connect4ClientSocket, &column, sizeof(column), 0);// Envoie le résultat au serveur qui jouera le coup
                            *yourTurn = SDL_FALSE;
                        } else {
                            SDL_Log("Coup Impossible ! \n"); // Erreur coup impossible
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                        setDisplayedFeedback("Ce n'est pas votre tour !"); // Erreur clique en dehors du tour
                    }
                } else {
                    SDL_Log("SDL EVENT RECEIVED %s DOES NOT MATCH REGEX",event->instructions); // Erreur regex match
                }
            } else if (event->type == NETWORK) { // Si le coup est de type NETWORK
                SDL_Log("CONNECT4 : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) { // Instruction de l'évenemnt est DISCONNECTED
                    SDL_Log("DECONNECTE DU SERVEUR");
                    setDisplayedInfo("DECONNECTE DU SERVEUR");
                    connect_launched = SDL_FALSE; // Met la le connect_launched dans SDL_FALSE
                } else if (strcmp("YOURTURN", event->instructions) == 0) { // Instruction de tour
                    SDL_Log("C'est à vous de jouer ! \n");
                    setDisplayedInfo("C'est à vous de jouer !");
                    *yourTurn = SDL_TRUE; // Indique pour votre tour
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) { // Instruction ENEMYTURN
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    setDisplayedInfo("C'est au tour de l'adversaire !");
                    *yourTurn = SDL_FALSE; // Indique le tour adverse
                } else if (regexec(&enemyPosRegex, event->instructions, 0, NULL, 0) == 0) {
                    int y;
                    sscanf(event->instructions, "ENEMY%d", &y); // Instruction coup de l'adversaire
                    tryPlay(game,y,RED); // Pose le coup dans le tableau
                    debugConnectBoard(game);
                    displayGameBoard(game); // Affiche le tableau
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) { // Instruction YOUWIN!!
                    SDL_Log("Vous avez gagné ! \n");
                    setDisplayedInfo("Vous avez gagné !"); //Indique la victoire
                    connect_launched = SDL_FALSE; // Déconnexion
                } else if (strcmp("YOULOSE!", event->instructions) == 0) { // Instruction YOULOSE!
                    SDL_Log("Vous avez perdu ! \n");
                    setDisplayedInfo("Vous avez perdu !"); // Indique la défaite
                    connect_launched = SDL_FALSE; // Déconnexion
                } else if (strcmp("EQUALITY", event->instructions) == 0) { // Instruction EQUALITY
                    SDL_Log("Personne n'a gagné ! \n");
                    setDisplayedInfo("Personne n'a gagné !"); // Inqdique l'égalité
                    connect_launched = SDL_FALSE; // Déconnexion
                } else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions); // Indique la confusion par rapport à un évenement SDL
                }
            }
            displayGameBoard(game);
            setDisplayedFeedback(" ");
        }

    }
    if(!connect4QuitForcedByPlayer){
        sleep(10);
    }
    quitSDL(rendererConnect4, windowConnect4);
    close(*socketClient);
    return 0;
}

void *sdlListener() { // Ecoute les évenements SDL afin qu'il puisse faire des actions
    while (connect_launched) { // tant que le jeu est connecté
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // tant qu'il y a des event
            switch (event.type) {
                case SDL_QUIT: // en en cas de fin de SDL
                    SDL_Log("SDL QUIT");
                    connect4QuitForcedByPlayer = SDL_TRUE;  // Force les joueurs a quitter
                    connect_launched = SDL_FALSE; // Met fin à la connection
                    break;
                case SDL_MOUSEBUTTONDOWN: // en cas de clique
                    ;
                    int column = calculateBoardColumnPos(event.button.x); // Garde la pose de X en fonction du clique fait sur le SDL
                    SDL_Log("SDL BTN DOWN");
                    NG_Event *buttonDown = malloc(sizeof(NG_Event));
                    if(buttonDown==NULL){
                        SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWN EVENT");
                    }
                    buttonDown->type = SDL;  // Définie le type d'évenement
                    buttonDown->instructions = malloc(sizeof(char)*11); // Alloue de la mémoire pou l'instruction
                    if(buttonDown->instructions==NULL){
                        SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWN INSTRUCTIONS");
                    }
                    sprintf(buttonDown->instructions, "POS%d", column);
                    sendEvent(buttonDown);  // Envoie au serveur l'evenement de click
                    break;
                default:
                    break;
            }
        }
    }
}



void *networkListener() { // Permet d'écouter les évenement recu du server
    NG_Event *disconnectEvent = malloc(sizeof(NG_Event)); // alloue la mémoire pour le NG_Event disconnectEvent déconection
    if(disconnectEvent==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT EVENT");
    }
    disconnectEvent->type = NETWORK; //définitions du type d'évenement pour disconnect
    disconnectEvent->instructions = malloc(sizeof(char)*12);
    if(disconnectEvent->instructions==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT INSTRUCTIONS");
    }
    disconnectEvent->instructions = "DISCONNECTED"; // définie l'instruction pour la déconnection

    char startData[6];
    memset(startData, '\0', sizeof(startData));
    do{
        if (recv(*connect4ClientSocket, startData, 5, 0) <= 0) {
            sendEvent(disconnectEvent);
            connect_launched = SDL_FALSE;
            break;
        } else {
            if(strcmp("START", startData) != 0)
                send(*connect4ClientSocket,"PONG",4,0);
        }
    } while(strcmp("START", startData) != 0);

    while (connect_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*connect4ClientSocket, data, 8, 0) <= 0) { // Si ll rrecois une fin du jeu par le serveur, alors il envoie disconnectEvent
            sendEvent(disconnectEvent);
            break;
        } else { // tant que il n'ya pas eu de déconnexion
            if (strcmp("WAITTURN", data) == 0) {
                //Création des différents évenements NETWORK

                NG_Event *enemyTurnEvent = malloc(sizeof(NG_Event)); // Evenement du tour adverse
                if(enemyTurnEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYTURN EVENT");
                }
                enemyTurnEvent->type = NETWORK; // définitions du type d'évenement
                enemyTurnEvent->instructions = malloc(sizeof(char)*10);
                if(enemyTurnEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYTURN INSTRUCTIONS");
                }
                enemyTurnEvent->instructions = "ENEMYTURN"; // définition de l'instruction du tour adverse
                sendEvent(enemyTurnEvent);
                int enemyPos;
                recv(*connect4ClientSocket, &enemyPos, sizeof(enemyPos), 0);


                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event));  // Evenement de placement d'un coup adverse
                if(enemyPosEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOS EVENT");
                }
                enemyPosEvent->type = NETWORK;  // définitions du type d'évenement
                enemyPosEvent->instructions = malloc(sizeof(char)*4); // Allour la mémoire
                if(enemyPosEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOS INSTRUCTIONS");
                }
                sprintf(enemyPosEvent->instructions, "ENEMY%d", enemyPos);
                sendEvent(enemyPosEvent);
            } else {

                NG_Event *receivedDataEvent = malloc(sizeof(NG_Event)); // Evenement reception de données
                if(receivedDataEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATA EVENT");
                }
                receivedDataEvent->type = NETWORK;  // définitions du type d'évenement
                unsigned long len = strlen(data);  // Prise de la longueur de la data
                SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data); // Ecrit un log sur ce paquet
                receivedDataEvent->instructions = malloc(sizeof(char)*len); // Allouer la mémoire en fonction de la longueur
                if(receivedDataEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATA INSTRUCTIONS");
                }
                strcpy(receivedDataEvent->instructions,data);
                sendEvent(receivedDataEvent);
            }
        }
    }
}


void setDisplayedInfo(char * text){
    actualDisplayedInfo = text;
}

void setDisplayedFeedback(char * text){
    actualDisplayedFeedback = text;
}


SDL_bool tryPlay(char **game, int column, char color) { // Essaie de pose d'un jetons dans une colonne
    if (column < 1 || column > 7) return SDL_FALSE;
    int count = SIZE - 1;
    while (game[column - 1][count] != '+') {
        count--;
    }
    if (count < 0) return SDL_FALSE;
    game[column - 1][count] = color;
    SDL_Log("[DEBUG] You placed your token on column %d, he fell to the line %d.\n", column, count + 1);
    return SDL_TRUE;
}

void debugConnectBoard(char **game) { // Affichage du board en débug
    SDL_Log("\n-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        SDL_Log("| %d ", i + 1);
    }
    SDL_Log("|\n");
    SDL_Log("-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            SDL_Log("| %c ", game[j][i]);
        }
        SDL_Log("|\n");
    }
    SDL_Log("-----------------------------\n");
}

int calculateBoardColumnPos(Sint32 x){ //retourne la hauteur en fonction de la position cliquer sur le SDL
    if(x>600){
        return 7;
    } else if(x>500){
        return 6;
    } else if(x>400){
        return 5;
    } else if(x>300){
        return 4;
    } else if(x>200){
        return 3;
    } else if(x>100){
        return 2;
    } else {
        return 1;
    }
}

void createSymbolCircle(char symbol, int x, int y){ // fonction de création de cercle en SDL
    int centerX = 50 + x * 100; // centrer l'axe x de la case en fonction de la colonne choisis
    int centerY = 150 + y * 100; // centrer l'axe x de la case en fonction de la colonne choisis
    if(symbol==BLUE){ // jeton du joueur en bleu
        changeColor(rendererConnect4,0,122,204);
        createCircle(rendererConnect4,centerX,centerY,30);
    } else if(symbol==RED){ // jetons du joueur en rouge
        changeColor(rendererConnect4,255,0,0);
        createCircle(rendererConnect4,centerX,centerY,30);
    }
}

void displayGameBoard(char **game) { // Création du tableau en SDL en créer chaque case et chaque lignes
    SDL_RenderClear(rendererConnect4);
    changeColor(rendererConnect4, 45,45,48); // met la couleur a blanc
    createFilledRectangle(0, 0, 700, 800, rendererConnect4); // Trace un rectangle
    changeColor(rendererConnect4, 0,122,204); // met la couleur a blanc
    createFilledRectangle(0, 100, 700, 5, rendererConnect4);
    createFilledRectangle(0, 795, 700, 5, rendererConnect4);
    createFilledRectangle(0, 100, 5, 700, rendererConnect4);
    createFilledRectangle(695, 100, 5, 700, rendererConnect4);

    createFilledRectangle(0, 195, 700, 10, rendererConnect4);
    createFilledRectangle(0, 295, 700, 10, rendererConnect4);
    createFilledRectangle(0, 395, 700, 10, rendererConnect4);
    createFilledRectangle(0, 495, 700, 10, rendererConnect4);
    createFilledRectangle(0, 595, 700, 10, rendererConnect4);
    createFilledRectangle(0, 695, 700, 10, rendererConnect4);

    createFilledRectangle(95, 100, 10, 700, rendererConnect4);
    createFilledRectangle(195, 100, 10, 700, rendererConnect4);
    createFilledRectangle(295, 100, 10, 700, rendererConnect4);
    createFilledRectangle(395, 100, 10, 700, rendererConnect4);
    createFilledRectangle(495, 100, 10, 700, rendererConnect4);
    createFilledRectangle(595, 100, 10, 700, rendererConnect4);

    if(actualDisplayedInfo!=NULL){ //Affiche un text seulement si celui-ci existe
        createTextZone(rendererConnect4,actualDisplayedInfo,200,10,0,150,0);
    }
    if(actualDisplayedFeedback!=NULL){ //Affiche un text seulement si celui-ci existe
        createTextZone(rendererConnect4,actualDisplayedFeedback,200,50,150,0,0);
    }


    for (int i = 0; i < SIZE; i++) { // Crée les symboles dans les case en fonction du tableau de jeu
        for (int j = 0; j < SIZE; j++) {
            createSymbolCircle(game[i][j], i, j);
        }
    }
    updateRenderer(rendererConnect4);
}


void initializeGame(char **game) { // Lancement d'une partie
    for (int i = 0; i < SIZE; i++) { // Allocation mémoire du tableau de jeu
        game[i] = malloc(sizeof(char) * SIZE);
        if(game[i]==NULL){
            SDL_ExitWithError("ERROR ALLOCATING game[i]");
        }
        for (int j = 0; j < SIZE; j++) { // Remplis le tableau avec des +
            game[i][j] = '+';
        }
    }
}
