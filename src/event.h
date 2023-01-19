// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

enum EventType
{
    EVENT_ANIMATION_COMPLETE,
    EVENT_COMBAT_OVER,
};

struct Event
{
    EventType type;

    Event(EventType type_in)
    : type(type_in)
    {}
};

static queue<Event> GlobalEvents;

void
EmitEvent(Event event)
{
    GlobalEvents.push(event);
}

void
HandleEvents(Fight *fight, Cursor *cursor)
{
    while(!GlobalEvents.empty())
    {
        Event event = GlobalEvents.front();
        GlobalEvents.pop();

        switch(event.type)
        {
            case EVENT_ANIMATION_COMPLETE:
            {
                fight->ready = true;
            } break;
            case EVENT_COMBAT_OVER:
            {
                if(GlobalPlayerTurn)
                {
                    GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                }
                else
                {
                    GlobalAIState = FINDING_NEXT;
                }
            } break;
            default:
            {
                assert(!"ERROR: Unimplemented Event\n");
            } break;
        }
    }
}

#endif
