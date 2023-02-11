// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

enum EventType
{
    START_GAME_EVENT,
    NEXT_LEVEL_EVENT,
    START_AI_TURN_EVENT,
    START_PLAYER_TURN_EVENT,
    MISSION_COMPLETE_EVENT,
    FADE_DONE_EVENT,
    MOVE_CURSOR_EVENT,
    MOVE_MENU_EVENT,
    SELECT_MENU_OPTION_EVENT,
    NEXT_SENTENCE_EVENT,
    PICK_UP_UNIT_EVENT,
    PLACE_UNIT_EVENT,
    UNIT_INFO_EVENT,
    ATTACK_HIT_EVENT,
    ATTACK_CRIT_EVENT,
    ATTACK_MISS_EVENT,
    ATTACK_RANGED_NOCK_EVENT,
    ATTACK_RANGED_HIT_EVENT,
    ATTACK_RANGED_MISS_EVENT,
    ATTACK_RANGED_CRIT_EVENT,
    BUFF_EVENT,
    HEAL_EVENT,
    DANCE_EVENT,
    EXPERIENCE_EVENT,
    EXPERIENCE_DONE_EVENT,
    ADVANCEMENT_EVENT,
    LEVEL_BOOST_EVENT,
    UNIT_DEATH_EVENT,
    UNIT_DEATH_OVER_EVENT,
};

struct Unit;
struct Event
{
    EventType type;
    Unit *unit = nullptr;
    int integer = 0;
    float number = 0.0f;

    Event(EventType type_in)
    : type(type_in) {}

    Event(EventType type_in,
          Unit *unit_in,
          int integer_in = 0,
          float number_in = 0.0f)
    : type(type_in),
      unit(unit_in),
      integer(integer_in),
      number(number_in)
    {}
};

static queue<Event> GlobalEvents;

void
EmitEvent(Event event)
{
    GlobalEvents.push(event);
}

#endif
