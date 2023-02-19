#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_log.h>
#include "EventManager.h"
#include "../sdl-utils/SDLUtils.h"

static void initQueue(NG_Queue ** queue);

static NG_Queue * networkReceivedQueue;
static NG_Queue * sdlQueue;


void eventManagerInit(){ // Permet de lancer l'event manager, qui gère la file des éléemnts SDL et NETOWRK
    initQueue(&networkReceivedQueue); // Lance la queue NETWORK
    initQueue(&sdlQueue); // Lance la queue SDL
}

static void initQueue(NG_Queue ** queue){  // Permet de lancer la queue
    *queue = malloc(sizeof(NG_Queue)); // Alloue l'espace mémoire pour la queue
    (*queue)->firstElement = malloc(sizeof(NG_Queue_Element)); // alloue un espace mémoire pour l'élément suivant 
    (*queue)->size = 0; // la taille a ce stade est 0
}

static void addToQueue(NG_Event * event, NG_Queue * queue){ // Ajoute un élément a la queue
    NG_Queue_Element * element = malloc(sizeof(NG_Queue_Element));
    element->event = event;
    element->next = NULL;
    // Si la firstElement n'est pas vide
    if(queue->size != 0){
        // Premier élément de la firstElement
        NG_Queue_Element * queueElement = queue->firstElement;
        // On récupère le dernier élément de la liste chainée et on le stocke dans queueElement
        while (queueElement->next!=NULL){
            queueElement = queueElement->next;
        }
        // On set l'élément précédant le nouvel élement ajouté
        element->previous = queueElement;
        // L'élément suivant le dernier élément actuel devient le nouvel élément ajouté
        queueElement->next = element;
    }
    // Si la firstElement est vide
    else {
        // L'élément précédent n'existe pas
        element->previous = NULL;
        queue->firstElement = element;
    }
    queue->size++;
}

void sendEvent(NG_Event * event){ // envoie d'un evènement au serveur
    if(event->type==SDL){ // si l'élément est de type SDL
        SDL_Log("EVENT SENT [SDL] : %s",event->instructions);
        addToQueue(event,sdlQueue); // ajoute a la queue SDL
    } else if(event->type == NETWORK){ // si l'élément est de type SDL
        SDL_Log("EVENT SENT [NETWORK] : %s",event->instructions);
        addToQueue(event, networkReceivedQueue); // Ajoute a la queue NETWORK
    }
}


static NG_Event * pickUpEvent(NG_Queue * queue){ // Prise en compte de l'évènement
    NG_Event  * ngEvent = queue->firstElement->event; // regarde la liste d'évènement et prend l'élement choisis
    if(ngEvent->type==SDL){ // met un log de l'évenement en fonction du type d'évenement reçu
        SDL_Log("EVENT PICKED UP [SDL] : %s",ngEvent->instructions);
    } else if (ngEvent->type==NETWORK){
        SDL_Log("EVENT PICKED UP [NETWORK] : %s",ngEvent->instructions);
    } else {
        SDL_Log("EVENT PICKED UP [NULLTYPE] : %s",ngEvent->instructions);
    }

    if(queue->firstElement->next!=NULL){ // si le prochain élément n'est pas null
        NG_Queue_Element * newFirstElement = queue->firstElement->next; // Crée le nouveau premier élément
        newFirstElement->previous = NULL; // Il n'as pas d'ancien élément
        NG_Queue_Element * oldFirstElement = queue->firstElement; // On récupère l'ancien poremier élément
        queue->firstElement = newFirstElement; // On met le nouveau premier élément en premier élément
        free(oldFirstElement); // on libère l'espace de l'ancien premier élément
    }
    queue->size--; // la queue decrémente car un élément a disparu
    return ngEvent;
}

void clearQueues(){ // vide la queue des évènements
    while(networkReceivedQueue->size!=0){ // tant qu'il y a des évènements NETWORK
        pickUpEvent(networkReceivedQueue); // Utilise les évènements NETWORK, mais ne les retournent pas
    }
    while(sdlQueue->size!=0){ // tant qu'il y a des évènements SDL
        pickUpEvent(networkReceivedQueue); // Utilise les évènements SDL, mais ne les rtournent pas
    }
}

NG_Event * listenAllEvents(){ // Ecoute les évènements et les retournent
    if(networkReceivedQueue->size != 0){ // tant qu'il y a des évènements NETWORK
        return pickUpEvent(networkReceivedQueue);  // Utilise les évènements NETWORK, et retournent les
    } else if(sdlQueue->size!=0){ // tant qu'il y a des évènements SDL
        return pickUpEvent(sdlQueue); // Utilise les évènements SDL, et retournent les
    }
    return NULL; // Sinon retourne NULL
}

NG_Event * createEvent(NG_EventType type, char * instructions){ // Crée un évènement
    NG_Event * event = malloc(sizeof(NG_Event)); // Alloue l'espace mémoire pour l'évènement 
    if(event==NULL){
       SDL_ExitWithError("Error while creating event");
    }
    event->type = type; // Choisis le type de l'évènement
    event->instructions = instructions; // Choisis l'instruction de l'évènement
    return event; // retourne l'évènement
}

