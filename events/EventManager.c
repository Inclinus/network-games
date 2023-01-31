#include <stdlib.h>
#include <stdio.h>
#include "EventManager.h"


NG_Queue * networkQueue;
NG_Queue * sdlQueue;

void eventManagerInit(){
    networkQueue = malloc(sizeof(NG_Queue));
    networkQueue->firstElement = malloc(sizeof(NG_Queue_Element));
    networkQueue->size = 0;
    sdlQueue = malloc(sizeof(NG_Queue));
    sdlQueue->firstElement = malloc(sizeof(NG_Queue_Element));
    sdlQueue->size = 0;
}

void addToQueue(NG_Event event, NG_Queue * queue){
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

void sendEvent(NG_Event event){
    if(event.type==SDL){
        printf("EVENT SENT [SDL] : %s",event.instructions);
        addToQueue(event,sdlQueue);
    } else if(event.type==NETWORK){
        printf("EVENT SENT [NETWORK] : %s",event.instructions);
        addToQueue(event,networkQueue);
    }
}

void removeElement(NG_Queue * queue, NG_Event * ngEvent){
    *ngEvent = queue->firstElement->event;
    printf("EVENT PICKED UP : %s",ngEvent->instructions);
    NG_Queue_Element * newFirstElement = queue->firstElement->next;
    newFirstElement->previous = NULL;
    free(queue->firstElement);
    queue->firstElement = newFirstElement;
    queue->size--;
}

int listenEventsOfType(NG_EventType type, NG_Event * ngEvent){
    if(type==SDL){
        if(sdlQueue->size!=0){
            removeElement(sdlQueue,ngEvent);
            return 1;
        } else return 0;
    } else if(type==NETWORK){
        if(networkQueue->size!=0){
            removeElement(networkQueue,ngEvent);
            return 1;
        } else return 0;
    }
    return 0;
}

int listenEvents(NG_Event * ngEvent){
    if(networkQueue->size!=0){
        removeElement(networkQueue,ngEvent);
        return 1;
    } else if(sdlQueue->size!=0){
        removeElement(sdlQueue,ngEvent);
        return 1;
    } else return 0;
}

