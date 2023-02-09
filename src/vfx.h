// Author: Alex Hartford
// Program: Emblem
// File: VFX

#ifndef VFX_H
#define VFX_H

// ============================== Transitions ==================================
struct Fade
{
    SDL_Color color = black;
    string text = "";
    Texture texture_one = {};
    Texture texture_two = {};
    float amount = 1.0f;
    Animation *animation = nullptr;
    bool show_first_texture = true;

    ~Fade()
    {
        delete animation;
        animation = nullptr;
    }

    Fade(string text_one, string text_two)
    {
        texture_one = LoadTextureText(text_one, blue, 0);
        texture_two = LoadTextureText(text_two, red, 0);
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
                        GlobalAIState = AI_RESOLVING_ADVANCEMENT;
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
    Boosts boosts = {};

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

                recipient->LevelUp(boosts);

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

#endif
