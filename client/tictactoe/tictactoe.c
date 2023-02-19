#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <regex.h>
#include "../../sdl-utils/SDLUtils.h"
#include "../../events/EventManager.h"
#include "../guis/MainMenu.h"
#include "../guis/ChoseGameMenu.h"

void displayBoard(int **board);

SDL_bool tryPlace(SDL_bool isEnemy, int ** board, int px, int py);
void placeSymbol(int ** board, int symbol, int px, int py);
void createSymbol(int symbol, int x, int y);

int calculateLinePos(Sint32 x);
int calculateColumnPos(Sint32 y);

void * sdlListen();
void *networkListen();

void setDisplayInfo(char * displayInfo);
void setDisplayFeedback(char * displayFeedback);

SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

SDL_bool program_launched = SDL_TRUE; // Booléen SDL pour garder la fenêtre ouverte

SDL_bool quitForcedByPlayer = SDL_FALSE; // Booléen SDL pour forcer la déconnexion

int * tictactoeClientSocket;

char * tictactoeDisplayInfo;
char * tictactoeDisplayFeedback;

int tictactoe(int * socketClient, SDL_Renderer * rendererMenu) {
    clearQueues();
    tictactoeClientSocket = socketClient;
    //window = SDL_CreateWindow("MORPION", 50, 50, 600, 700, 0); // Création de la fenêtre
    renderer = rendererMenu;

    pthread_t network_listener; // Création d'un thread pour les évenement NETWORK
    pthread_create(&network_listener, NULL, networkListen, NULL);
    pthread_t sdl_listener; // Création d'un thread pour les évenement SDL
    pthread_create(&sdl_listener, NULL, sdlListen, NULL);

    int **board;
    int *row;
    board = malloc(sizeof(int *) * 3); // Création du tableau de jeu
    if(board==NULL){
        SDL_ExitWithError("ERROR ALLOCATING BOARD");
    }

    for (int i = 0; i < 3; ++i) { // Création de la 2ème dimension du tableau
        row = malloc(sizeof(int) * 3); // Allocation mémoire
        if(row==NULL){
            SDL_ExitWithError("ERROR ALLOCATING ROW");
        }
        board[i] = row;
    }
    for (int i = 0; i < 3; ++i) { // remplissage du tableau de 0
        for (int y = 0; y < 3; ++y) {
            board[i][y] = 0;
        }
    }
    displayBoard(board);

    regex_t posRegex; // Regex qui vérifie une instruction de coup joué
    if (regcomp(&posRegex, "^[0-9]-[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) { // Vérifie la string avec la regex
        fprintf(stderr, "Error: Could not compile regular expression\n");
        return 1;
    }

    SDL_bool yourTurn = SDL_FALSE; // Booléen pour le tour de jeu

    while (program_launched) { // Tant que le program est lancé
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) { // Tant que les évenement sont différent de NULL
            SDL_Log("TICTACTOE : EVENT RECEIVED");
            if (event->type == SDL) { // Si l'évenement est de type SDL
                SDL_Log("TICTACTOE : SDL EVENT RECEIVED %s",event->instructions);
                if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) { // Instruction pour un coup
                    if(yourTurn==SDL_TRUE){ // Si c'est votre tour
                        int x, y;
                        sscanf(event->instructions, "%d-%d", &x, &y); // Prend les coordonnées
                        if(tryPlace(SDL_FALSE,board,x,y)){ // Si le coup est possible
                            SDL_Log("Coup Possible ! \n");
                            send(*socketClient, &x, sizeof(x), 0); // Envoie les coordonnées X au serveur
                            send(*socketClient, &y, sizeof(y), 0); // Envoie les coordonnées Y au serveur
                            yourTurn = SDL_FALSE; // Fin de tour
                        } else {
                            SDL_Log("Coup Impossible ! \n");
                            setDisplayFeedback("Coup Impossible !");
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                        setDisplayFeedback("Ce n'est pas votre tour !");
                    }
                }
            } else if (event->type == NETWORK) {  // Si l'évenement est de type NETWORK
                SDL_Log("TICTACTOE : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) { // Instruction DISCONNECTED
                    SDL_Log("DECONNECTE DU SERVEUR");
                    setDisplayInfo("DECONNECTE DU SERVEUR"); // Affiche l'information de déconnexion au joueur
                    program_launched = SDL_FALSE; // Met fin au programme
                } else if (strcmp("YOURTURN", event->instructions) == 0) { // Instruction YOURTURN
                    SDL_Log("C'est à vous de jouer ! \n");
                    setDisplayInfo("C'est à vous de jouer !"); // Affiche l'info au joueur
                    yourTurn = SDL_TRUE; // Début de votre tour
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) { // Instruction ENEMYTURN
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    setDisplayInfo("C'est au tour de l'adversaire !"); // Affiche l'info au joueur
                    yourTurn = SDL_FALSE; // Fin de tour
                } else if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) { // Instruction pour un coup
                    int x, y;
                    sscanf(event->instructions, "%d-%d", &x, &y); // Prend les coordonnées de jeu
                    placeSymbol(board, 2, x, y); // Place le symbol dans le tableau
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) { // Instruction YOUWIN!!
                    SDL_Log("Vous avez gagné ! \n");
                    setDisplayInfo("Vous avez gagné !"); // Affiche le résultat
                    program_launched = SDL_FALSE; // Ferme le programme
                } else if (strcmp("YOULOSE!", event->instructions) == 0) { // Instruction YOULOSE!
                    SDL_Log("Vous avez perdu ! \n");
                    setDisplayInfo("Vous avez perdu !"); // Affiche le résultat
                    program_launched = SDL_FALSE; // Ferme le programme
                }  else if (strcmp("DRAWDRAW", event->instructions) == 0) { // Instruction DRAWDRAW
                    SDL_Log("Personne n'a gagné ! \n");
                    setDisplayInfo("Personne n'a gagné !"); // Affiche le résultat
                    program_launched = SDL_FALSE; // Ferme le programme
                }
                else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions); // Confusion par rapport à l'évenement SDL qui est inconnu
                }
            }
            displayBoard(board);
            setDisplayFeedback(" ");
        }
    }
    if(!quitForcedByPlayer){
        sleep(5);
    }
    return 0;
}

void *sdlListen() { // Ecoute des évenements SDL
    while (program_launched) { // Tanst que le program est lancé
        SDL_Event event;

        while (SDL_PollEvent(&event)) { // Tant qu'il y a des évenement
            switch (event.type) { // Switch des évenement
                case SDL_QUIT: // Event SDL_QUIT
                    SDL_Log("SDL QUIT");
                    quitForcedByPlayer = SDL_TRUE;
                    program_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN: // Event SDL_MOUSEBUTTONDOWN
                    if(event.button.x > 179 && event.button.x < 541 && event.button.y > 119 && event.button.y < 480){
                        SDL_Log("SDL BTN DOWN");
                        int x = calculateLinePos(event.button.x); // Calcul la ligne en fonction de l'axe X du clique
                        int y = calculateColumnPos(event.button.y); // Calcul la ligne en fonction de l'axe Y du clique
                        NG_Event *buttonDown = malloc(sizeof(NG_Event)); // Création de l'évenemnt buttonDown
                        if (buttonDown == NULL) { // Erreur si il est NULL
                            SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWNEVENT");
                        }
                        buttonDown->type = SDL; // Type de l'evenement buttonDown
                        buttonDown->instructions = malloc(sizeof(char) * 11); // Allorcation mémoire pour l'instruction
                        if (buttonDown->instructions == NULL) {
                            SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWNEVENT INSTRUCTIONS");
                        }
                        sprintf(buttonDown->instructions, "%d-%d", x, y); // Met en forme les coordonnées
                        sendEvent(buttonDown); // Envoie les coordonnées au serveur

                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void *networkListen() { // Ecoute des évenements NETWORK
    NG_Event *disconnectEvent = malloc(sizeof(NG_Event)); // Créations de l'évènement disconnectEvent
    if(disconnectEvent==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT");
    }
    disconnectEvent->type = NETWORK; // Type de l'évènement disconnectEvent
    disconnectEvent->instructions = malloc(sizeof(char)*12); // Allocation mémoire de l'instruction disconnectEvent
    if(disconnectEvent->instructions==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT INSTRUCTIONS");
    }
    disconnectEvent->instructions = "DISCONNECTED"; // Instruction de disconnectEvent

    char startData[6];
    memset(startData, '\0', sizeof(startData));
    do{
        if (recv(*tictactoeClientSocket, startData, 5, 0) <= 0) {
            sendEvent(disconnectEvent);
            program_launched = SDL_FALSE;
            break;
        } else {
            if(strcmp("START", startData) != 0)
                send(*tictactoeClientSocket,"PONG",4,0);
        }
    } while(strcmp("START", startData) != 0);

    while (program_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*tictactoeClientSocket, data, 8, 0) <= 0) { // En cas de connexion interrompu, se déconnecte
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("WAITTURN", data) == 0) {
                sendEvent(createEvent(NETWORK,"ENEMYTURN"));
                int px;
                int py;
                recv(*tictactoeClientSocket, &px, sizeof(px), 0);
                SDL_Log("[TICTACTOE NETWORK_LISTENER] PACKET RECEIVED [1] - CONTENT: \"%d\"",px);
                recv(*tictactoeClientSocket, &py, sizeof(py), 0);
                SDL_Log("[TICTACTOE NETWORK_LISTENER] PACKET RECEIVED [2] - CONTENT: \"%d\"",py);
                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event)); // enemyPosEvent = Oxeaf & *enemyPosEvent = NG_EVENT{} &enemyPosEvent = 0xfk
                if(enemyPosEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOSEVENT");
                }
                enemyPosEvent->type = NETWORK; // Type de l'évènement enemyPosEvent
                enemyPosEvent->instructions = malloc(sizeof(char)*4); // Allocation mémoire de l'instruction de enemyPosEvent
                if(enemyPosEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOSEVENT INSTRUCTIONS");
                }
                sprintf(enemyPosEvent->instructions, "%d-%d", px, py);
                sendEvent(enemyPosEvent);
            } else {
                NG_Event *receivedDataEvent = malloc(sizeof(NG_Event)); // Créations de l'évènement receivedDataEvent
                if(receivedDataEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT");
                }
                receivedDataEvent->type = NETWORK; // Type de l'évènement disconnectEvent
                unsigned long len = strlen(data); // Prise de la longeur de la string de data
                SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
                receivedDataEvent->instructions = malloc(sizeof(char)*len); // allocation de l'espace mémoire pour l'instruction
                if(receivedDataEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT INSTRUCTIONS");
                }
                strcpy(receivedDataEvent->instructions,data); // Copie de la string de data dans l'instructiob
                sendEvent(receivedDataEvent); // Envoie l'event au serveur
            }
        }
    }
}

void setDisplayInfo(char * displayInfo){ // Controle la variable displayInfo
    tictactoeDisplayInfo = displayInfo;
}
void setDisplayFeedback(char * displayFeedback){ // Contrôle la variable displayFeedback
    tictactoeDisplayFeedback = displayFeedback;
}


SDL_bool tryPlace(SDL_bool isEnemy, int ** board, int px, int py){ // Placage d'un coup sur le jeu
    if(board[px][py]!=1 && board[px][py]!=2 && px > -1 && px < 3 && py > -1 && py < 3) { // Si le coup a des coordonnées correctes
        if(isEnemy){ // Vérfie quel signe il faut placer
            placeSymbol(board,2,px,py);
        } else {
            placeSymbol(board,1,px,py);
        }
        return SDL_TRUE; // Indique que le coup est valide
    } else {
        return SDL_FALSE; // Le coup n'est pas valide
    }
}

void displayBoard(int **board) { // Affiche le plateau en SDL

    // WIDTH : 720
    // HEIGHT : 480

    // RECTANGLE DE JEU =
    //  BEGINX = 180
    //  BEGINY = 120
    //  ENDX = 540
    //  ENDY = 480


    SDL_RenderClear(renderer); // Nettoie le rendu
    changeColor(renderer, 45,45,48); // Met la couleur blanche
    createFilledRectangle(0, 0, 600, 700, renderer); // Met un fond blanc

    changeColor(renderer, 45,95,48); // Met la couleur du fond de jeu
    createFilledRectangle(180, 120, 360, 360, renderer); // Crée le fond de jeu

    changeColor(renderer, 0,122,204); // Met la couleur noir
    createFilledRectangle(180, 235, 360, 10, renderer); // Crée les délimitations de jeu
    createFilledRectangle(180, 355, 360, 10, renderer);
    createFilledRectangle(295, 120, 10, 360, renderer);
    createFilledRectangle(415, 120, 10, 360, renderer);

    if(tictactoeDisplayInfo!=NULL){ // Affiche le texte si displayInfo à une valeur
        createTextZone(renderer,tictactoeDisplayInfo,200,10,0,150,0);
    }
    if(tictactoeDisplayFeedback!=NULL){ // Affiche le texte si displayFeedback à une valeur
        createTextZone(renderer,tictactoeDisplayFeedback,200,50,150,0,0);
    }

    for (int i = 2; i >= 0; --i) { // Affiche les symboles sur le rendu du jeu
        for (int y = 0; y < 3; ++y) {
            createSymbol(board[i][y],i,y);
        }
    }
    updateRenderer(renderer); // met a jour le rendu du jeu
}

void placeSymbol(int **board, int symbol, int px, int py) { // Place les coordonnées
    board[px][py] = symbol; // Change le symbole pour ces coordonnées
}

void createSymbol(int symbol, int x, int y){ // Crée le symbole en SDL
    /*
     * 97-97    290-97    502-97
     * 97-290   290-290   502-290
     * 97-502   290-502   502-502
     */

    /**
     * 240-180  360-180  480-180
     * 240-300  360-300  480-300
     * 240-420  360-420  480-420
     */
    int centerX = 240 + x * 120; // Définie le centre X de la case
    int centerY = 180 + y * 120; // Définie le centre Y de la case
    if(symbol==1){ // Le symbole à mettre est un rond
        changeColor(renderer,0,0,255); // Choisis la couleur
        createCircle(renderer,centerX,centerY,15); // Crée le cercle
    } else if(symbol==2){ // Le symbole à mettre est une croix
        changeColor(renderer,255,0,0); // Changement de couleur
        SDL_RenderDrawLine(renderer,centerX-20,centerY-20,centerX+20,centerY+20); // Crée la croix
    }
}

int calculateLinePos(Sint32 x){ // Trouve la colonne choisis en SDL grace au coordonnées X
    if(x>420){
        return 2;
    } else if(x<300){
        return 0;
    } else {
        return 1;
    }
}

int calculateColumnPos(Sint32 y){// Trouve la ligne choisis en SDL grace au coordonnées Y
    if(y>360){
        return 2;
    } else if(y<240){
        return 0;
    } else {
        return 1;
    }
}


