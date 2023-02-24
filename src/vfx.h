// Author: Alex Hartford
// Program: Emblem
// File: VFX

#ifndef VFX_H
#define VFX_H

void
EventSystemUpdate()
{
    while(!GlobalEvents.empty())
    {
        Event event = GlobalEvents.front();
        GlobalEvents.pop();

        switch(event.type)
        {
            case START_GAME_EVENT:
            {
                PlaySfx("mission_start.wav");
            } break;
            case NEXT_LEVEL_EVENT:
            {
                PlaySfx("mission_start.wav");
            } break;
            case END_TURN_EVENT:
            {
                PlaySfx("mission_start.wav");
            } break;
            case MISSION_COMPLETE_EVENT:
            {
                PlaySfx("mission_complete.wav");
            } break;
            case MOVE_CURSOR_EVENT:
            {
                PlaySfx("click.wav");
            } break;
            case MOVE_MENU_EVENT:
            {
                PlaySfx("sel2.wav");
            } break;
            case SELECT_MENU_OPTION_EVENT:
            {
                PlaySfx("pitch.wav");
            } break;
            case PICK_UP_UNIT_EVENT:
            {
                PlaySfx("pickup.wav");
            } break;
            case PLACE_UNIT_EVENT:
            {
                PlaySfx("place.wav");
            } break;
            case UNIT_INFO_EVENT:
            {
                PlaySfx("sel2.wav");
            } break;
            case ATTACK_HIT_EVENT:
            {
                PlaySfx("hit1.wav");
            } break;
            case ATTACK_RANGED_NOCK_EVENT:
            {
                PlaySfx("nock.wav");
            } break;
            case ATTACK_RANGED_HIT_EVENT:
            {
                PlaySfx("ranged.wav");
            } break;
            case HEAL_EVENT:
            {
                PlaySfx("heal.wav");
            } break;
            case DANCE_EVENT:
            {
                PlaySfx("dance.wav");
            } break;
        }
    }
}

#endif
