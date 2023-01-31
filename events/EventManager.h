typedef enum {
    SDL = 0,
    NETWORK = 1
} NG_EventType;

typedef struct NG_Event{
    NG_EventType type;
    char * instructions;
} NG_Event;

typedef struct NG_Queue_Element {
    struct NG_Event event;
    struct NG_Queue_Element * next;
    struct NG_Queue_Element * previous;
} NG_Queue_Element;

typedef struct NG_Queue{
    struct NG_Queue_Element * firstElement;
    int size;
} NG_Queue;

void eventManagerInit();
void sendEvent(NG_Event event);
int listenEventsOfType(NG_EventType type, NG_Event * ngEvent);
int listenEvents(NG_Event * ngEvent);
