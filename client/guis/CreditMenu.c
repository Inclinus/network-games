#include "CreditMenu.h"
#include "MainMenu.h"

SDL_bool * creditMenuRunning = NULL;

void creditMenu(SDL_Renderer * rendererMenu){ // Menu des crédit
    creditMenuRunning = malloc(sizeof(SDL_bool));
    if(creditMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CREDITMENURUNNING SDLBOOL");
    }
    *creditMenuRunning = SDL_TRUE;

    Button option; // Créations de paramètre du boutton option
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu); // Clear le rendu SDL

    createTextZoneCentered(rendererMenu, "Credit : ",WIDTH/2,50, 255, 255, 255,48); // Titre du texte

    createTextZoneCentered(rendererMenu, "Noam DE MASURE",WIDTH/2,HEIGHT/2-60, 255, 255, 255,48);
    createTextZoneCentered(rendererMenu, "Thibaut LULINSKI ",WIDTH/2,HEIGHT/2, 255, 255, 255,48);
    createTextZoneCentered(rendererMenu, "Alexandre COMET ",WIDTH/2,HEIGHT/2+60, 255, 255, 255,48);

    createButton(rendererMenu,option, "retour");

    updateRenderer(rendererMenu); // Mise à jour du rendu

    while(*creditMenuRunning){ // tant que le menu est actif
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT: // Quitte en cas d'évenemnt de type SDL_QUIT
                    *creditMenuRunning = SDL_TRUE;
                    break;
                case SDL_MOUSEBUTTONDOWN: // En cas de clique de souris
                    ;
                    int x = event.button.x; // Prise des coordonnées de X
                    int y = event.button.y; // Prise des coordonnées de Y
                    if(x>option.beginX && x<option.endX && y<option.endY && y>option.beginY){ // Si le clique est sur le bouton retour
                        *creditMenuRunning = SDL_FALSE; // fin du menu actuel
                        loadMainMenu(); // retourne au menu principale
                    }
                    else{
                        continue;
                    }
                default:
                    break;
            }
        }
    }
}