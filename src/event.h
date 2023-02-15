// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

enum EventType
{
    START_GAME_EVENT,
    NEXT_LEVEL_EVENT,
    END_TURN_EVENT,
    MISSION_COMPLETE_EVENT,
    MOVE_CURSOR_EVENT,
    MOVE_MENU_EVENT,
    SELECT_MENU_OPTION_EVENT,
    PICK_UP_UNIT_EVENT,
    PLACE_UNIT_EVENT,
    UNIT_INFO_EVENT,
    ATTACK_HIT_EVENT,
    ATTACK_RANGED_NOCK_EVENT,
    ATTACK_RANGED_HIT_EVENT,
    HEAL_EVENT,
    DANCE_EVENT,
};

struct Event
{
    EventType type;

    Event(EventType type_in)
    : type(type_in) {}
};

static queue<Event> GlobalEvents;

void
EmitEvent(Event event)
{
    GlobalEvents.push(event);
}

#endif
