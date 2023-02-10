// Author: Alex Hartford
// Program: Emblem
// File: Event

#ifndef EVENT_H
#define EVENT_H

void
GlobalHandleEvents(Fade *level_fade,
                   Fade *turn_fade,
                   Parcel *parcel,
                   Advancement *advancement
                   )
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
                level_fade->amount = 1.0f;
                level_fade->animation = GetAnimation(FADE_OUT_ANIMATION);
            } break;
            case NEXT_LEVEL_EVENT:
            {
                PlaySfx("mission_start.wav");
                level_fade->amount = 1.0f;
                level_fade->animation = GetAnimation(FADE_OUT_ANIMATION);
            } break;
            case START_AI_TURN_EVENT:
            {
                PlaySfx("mission_start.wav");
                turn_fade->amount = 0.0f;
                turn_fade->animation = GetAnimation(TURN_FADE_IN_OUT_ANIMATION);
                turn_fade->show_first_texture = false;
            } break;
            case START_PLAYER_TURN_EVENT:
            {
                PlaySfx("mission_start.wav");
                turn_fade->amount = 0.0f;
                turn_fade->animation = GetAnimation(TURN_FADE_IN_OUT_ANIMATION);
                turn_fade->show_first_texture = true;
            } break;
            case MISSION_COMPLETE_EVENT:
            {
                PlaySfx("mission_complete.wav");
            } break;
            case FADE_DONE_EVENT:
            {
                if(GlobalInterfaceState == NO_OP && GlobalPlayerTurn)
                    GlobalInterfaceState = NEUTRAL_OVER_UNIT;
                if(GlobalAIState == AI_NO_OP && !GlobalPlayerTurn)
                    GlobalAIState = AI_FINDING_NEXT;
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
                PlaySfx("whoosh.wav");
                PlaySfx("miss.wav");
            } break;
            case ATTACK_RANGED_NOCK_EVENT:
            {
                PlaySfx("nock.wav");
            } break;
            case ATTACK_RANGED_HIT_EVENT:
            {
                PlaySfx("ranged.wav");
            } break;
            case ATTACK_RANGED_MISS_EVENT:
            {
                PlaySfx("whoosh.wav");
                PlaySfx("miss.wav");
            } break;
            case ATTACK_RANGED_CRIT_EVENT:
            {
                PlaySfx("crit.wav");
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
                parcel->animation = GetAnimation(EXPERIENCE_PARCEL_ANIMATION, event.number);
                parcel->recipient = event.unit;
                parcel->total = event.integer;
                PlaySfx("experience.wav");
            } break;
            case EXPERIENCE_DONE_EVENT:
            {
                StopSfx("experience.wav");
            } break;
            case ADVANCEMENT_EVENT:
            {
                advancement->animation = GetAnimation(ADVANCEMENT_ANIMATION);
                advancement->recipient = event.unit;
                advancement->boosts = advancement->recipient->CalculateLevelUp();
                PlaySfx("levelup.wav");
            } break;
            default:
            {
                cout << "WARN: Unimplemented Event\n";
            } break;
            case LEVEL_BOOST_EVENT:
            {
                PlaySfx("boost.wav");
            } break;
        }
    }
}

#endif
