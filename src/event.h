// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

void
GlobalHandleEvents(Fade *level_fade,
                   Fade *turn_fade,
                   Parcel *parcel)
{
    while(!GlobalEvents.empty())
    {
        Event event = GlobalEvents.front();
        GlobalEvents.pop();

        switch(event.type)
        {
            case START_GAME_EVENT:
            {
                PlaySfx("start.wav");
                level_fade->amount = 1.0f;
                level_fade->animation = GetAnimation(FADE_OUT_ANIMATION);
            } break;
            case NEXT_LEVEL_EVENT:
            {
                PlaySfx("start.wav");
                level_fade->amount = 1.0f;
                level_fade->animation = GetAnimation(FADE_OUT_ANIMATION);
            } break;
            case END_PLAYER_TURN_EVENT:
            {
                PlaySfx("start.wav");
                turn_fade->amount = 0.0f;
                turn_fade->animation = GetAnimation(FADE_IN_OUT_ANIMATION, 0.25f);
            }
            case END_AI_TURN_EVENT:
            {
                PlaySfx("start.wav");
                turn_fade->amount = 0.0f;
                turn_fade->animation = GetAnimation(FADE_IN_OUT_ANIMATION, 0.25f);
            }
            case MOVE_CURSOR_EVENT:
            {
                PlaySfx("mov.wav");
            } break;
            case MOVE_MENU_EVENT:
            {
                PlaySfx("sel2.wav");
            } break;
            case SELECT_MENU_OPTION_EVENT:
            {
                PlaySfx("sel1.wav");
            } break;
            case NEXT_SENTENCE_EVENT:
            {
                PlaySfx("sel3.wav");
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
            case ATTACK_CRIT_EVENT:
            {
                PlaySfx("crit.wav");
            } break;
            case ATTACK_MISS_EVENT:
            {
                PlaySfx("miss04.wav");
            } break;
            case ATTACK_RANGED_EVENT:
            {
                PlaySfx("ranged.wav");
            } break;
            case BUFF_EVENT:
            {
                PlaySfx("powerup.wav");
            } break;
            case HEAL_EVENT:
            {
                PlaySfx("heal.wav");
            } break;
            case DANCE_EVENT:
            {
                PlaySfx("dance.wav");
            } break;
            case EXPERIENCE_EVENT:
            {
                parcel->animation = GetAnimation(EXPERIENCE_PARCEL_ANIMATION);
                parcel->recipient = event.unit;
                parcel->total = event.integer;
                //PlaySfx("dance.wav"); // TODO: Get the actual exp bar sound
            } break;
            default:
            {
                cout << "WARN: Unimplemented Event\n";
            } break;
        }
    }
}

#endif
