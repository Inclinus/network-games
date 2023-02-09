#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_log.h>
#include "EventManager.h"

static void initQueue(NG_Queue ** queue);

static NG_Queue * networkReceivedQueue;
static NG_Queue * sdlQueue;


void eventManagerInit(){
    initQueue(&networkReceivedQueue);
    initQueue(&sdlQueue);
}

static void initQueue(NG_Queue ** queue){
    *queue = malloc(sizeof(NG_Queue));
    (*queue)->firstElement = malloc(sizeof(NG_Queue_Element));
    (*queue)->size = 0;
}

static void addToQueue(NG_Event * event, NG_Queue * queue){
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

void sendEvent(NG_Event * event){
    if(event->type==SDL){
        SDL_Log("EVENT SENT [SDL] : %s",event->instructions);
        addToQueue(event,sdlQueue);
    } else if(event->type == NETWORK){
        SDL_Log("EVENT SENT [NETWORK] : %s",event->instructions);
        addToQueue(event, networkReceivedQueue);
    }
}


static NG_Event * pickUpEvent(NG_Queue * queue){
    NG_Event  * ngEvent = queue->firstElement->event;
    if(ngEvent->type==SDL){
        SDL_Log("EVENT PICKED UP [SDL] : %s",ngEvent->instructions);
    } else if (ngEvent->type==NETWORK){
        SDL_Log("EVENT PICKED UP [NETWORK] : %s",ngEvent->instructions);
    } else {
        SDL_Log("EVENT PICKED UP [NULLTYPE] : %s",ngEvent->instructions);
    }

    if(queue->firstElement->next!=NULL){
        NG_Queue_Element * newFirstElement = queue->firstElement->next;
        newFirstElement->previous = NULL;
        NG_Queue_Element * oldFirstElement = queue->firstElement;
        queue->firstElement = newFirstElement;
        free(oldFirstElement);
    }
    queue->size--;
    return ngEvent;
}

void clearQueues(){
    while(networkReceivedQueue->size!=0){
        pickUpEvent(networkReceivedQueue);
    }
    while(sdlQueue->size!=0){
        pickUpEvent(networkReceivedQueue);
    }
}

NG_Event * listenAllEvents(){
    if(networkReceivedQueue->size != 0){
        return pickUpEvent(networkReceivedQueue);
    } else if(sdlQueue->size!=0){
        return pickUpEvent(sdlQueue);
    }
    return NULL;
}

