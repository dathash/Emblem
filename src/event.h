// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

enum EventType
{
    MOVE_CURSOR,
    PICK_UP_UNIT,
    PLACE_UNIT,
    SELECT_MENU_OPTION,
    MOVE_MENU,
    ATTACK_HIT,
    ATTACK_CRIT,
    ATTACK_MISS,
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
GlobalHandleEvents()
{
    while(!GlobalEvents.empty())
    {
        Event event = GlobalEvents.front();
        GlobalEvents.pop();

        switch(event.type)
        {
            case MOVE_CURSOR:
            {
                PlaySfx("mov.wav");
            } break;
            case SELECT_MENU_OPTION:
            {
                PlaySfx("sel1.wav");
            } break;
            case MOVE_MENU:
            {
                PlaySfx("sel2.wav");
            } break;
            case PICK_UP_UNIT:
            {
                PlaySfx("pickup.wav");
            } break;
            case PLACE_UNIT:
            {
                PlaySfx("place.wav");
            } break;
            case ATTACK_HIT:
            {
                PlaySfx("hit1.wav");
            } break;
            case ATTACK_CRIT:
            {
                PlaySfx("crit.wav");
            } break;
            case ATTACK_MISS:
            {
                PlaySfx("miss.wav");
            } break;
            default:
            {
                assert(!"ERROR: Unimplemented Event\n");
            } break;
        }
    }
}

#endif
