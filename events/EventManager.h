typedef enum {
    SDL = 0,
    NETWORK = 1
} NG_EventType;

typedef struct NG_Event{
    NG_EventType type;
    char * instructions;
} NG_Event;

typedef struct NG_Queue_Element {
    struct NG_Event * event;
    struct NG_Queue_Element * next;
    struct NG_Queue_Element * previous;
} NG_Queue_Element;

typedef struct NG_Queue{
    struct NG_Queue_Element * firstElement;
    int size;
} NG_Queue;

// INIT : MUST BE CALLED BEFORE ALL INTERACTIONS WITH THE EVENTMANAGER
void eventManagerInit();

// Send an event
void sendEvent(NG_Event * event);

// Clear queues
void clearQueues();

// Listen for NETWORK and SDL events
NG_Event * listenAllEvents();


// Create NG_Event
NG_Event * createEvent(NG_EventType type, char * instructions);