// Author: Alex Hartford
// Program: Emblem
// File: VFX

#ifndef VFX_H
#define VFX_H

// ============================== Transitions ==================================
enum FadeType
{
    FADE_LEVEL,
    FADE_TURN,
};

struct Fade
{
    FadeType type = FADE_LEVEL;
    SDL_Color color = black;
    string text = "";
    Texture texture_one = {};
    Texture texture_two = {};
    Texture turn_count_texture = {};
    float amount = 1.0f;
    Animation *animation = nullptr;
    bool show_first_texture = true;

    ~Fade()
    {
        delete animation;
        animation = nullptr;
    }

    Fade(string text_one, string text_two, FadeType type_in = FADE_LEVEL)
    : type(type_in)
    {
        texture_one = LoadTextureText(text_one, blue, 0);
        texture_two = LoadTextureText(text_two, red, 0);
        turn_count_texture = LoadTextureText("1", white, 0);
    }

    void
    Update()
    {
        if(animation)
        {
            amount = animation->Value(CHANNEL_ONE);
            if(animation->Update())
            {
                delete animation;
                animation = nullptr;
                amount = 1.0f;
                if(type == FADE_TURN)
                    EmitEvent(FADE_DONE_EVENT);
            }
        }
    }
};


// ============================ Experience UI ==================================
struct Parcel
{
    Unit *recipient = nullptr;
    float value = 0.0f;
    int total = 0;
    Animation *animation = nullptr;

    ~Parcel()
    {
        delete animation;
    }

    int
    Amount() const
    {
        return (recipient->experience + (int)(value * total)) % 100;
    }

    void
    Update()
    {
        if(animation)
        {
            value = animation->Value(CHANNEL_ONE);

            if(animation->Update())
            {
                delete animation;
                animation = nullptr;

                if(recipient->GrantExperience(total))
                {
                    EmitEvent({ADVANCEMENT_EVENT, recipient, 0});
                    if(GlobalPlayerTurn)
                        GlobalInterfaceState = RESOLVING_ADVANCEMENT;
                    else
                        GlobalAIState = AI_NO_OP;
                }
                else if(GlobalInterfaceState != LEVEL_MENU)
                {
                    if(GlobalPlayerTurn)
                        GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                    else
                        GlobalAIState = AI_FINDING_NEXT;
                }

                recipient = nullptr;
                value = 0.0f;
                total = 0;

                return;
            }
        }
    }
};


struct Advancement
{
    Unit *recipient = nullptr;
    float value = 0.0f;
    Animation *animation = nullptr;

    ~Advancement()
    {
        delete animation;
    }

    void
    Update()
    {
        if(animation)
        {
            value = animation->Value(CHANNEL_ONE);

            if(animation->Update())
            {
                delete animation;
                animation = nullptr;

                if(GlobalInterfaceState != LEVEL_MENU)
                {
                    if(GlobalPlayerTurn)
                        GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                    else
                        GlobalAIState = AI_FINDING_NEXT;
                }
                recipient = nullptr;
                value = 0.0f;
                return;
            }
        }
    }
};

void
EventSystemUpdate(Fade *level_fade,
                  Fade *turn_fade,
                  Parcel *parcel,
                  Advancement *advancement,
                  Unit **dying
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
                turn_fade->turn_count_texture = LoadTextureText("1", white, 0);
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

                if(event.integer) // Increment turn display
                    turn_fade->turn_count_texture = LoadTextureText(to_string(event.integer), white, 0);
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
                PlaySfx("levelup.wav");
            } break;
            case UNIT_DEATH_EVENT:
            {
                PlaySfx("levelup.wav");
                *dying = event.unit;
            } break;
            case UNIT_DEATH_OVER_EVENT:
            {
                StopSfx("levelup.wav");
                if(GlobalPlayerTurn)
                {
                    GlobalInterfaceState = NEUTRAL_OVER_GROUND;
                    GlobalAIState = AI_PLAYER_TURN;
                }
                else
                {
                    GlobalInterfaceState = NO_OP;
                    GlobalAIState = AI_FINDING_NEXT;
                }
                (*dying)->should_die = true;
                (*dying)->has_spoken_valediction = true;
                *dying = nullptr;
            } break;
            default:
            {
                cout << "WARN: Unimplemented Event " << event.type << "\n";
            } break;
            case LEVEL_BOOST_EVENT:
            {
                PlaySfx("boost.wav");
            } break;
        }
    }
}

#endif
