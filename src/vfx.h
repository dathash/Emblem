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
    float amount = 0.0f;
    Animation *animation = nullptr;

    ~Fade()
    {
        delete animation;
        animation = nullptr;
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

                recipient->GrantExperience(total);

                if(GlobalPlayerTurn)
                    GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                else
                    GlobalAIState = FINDING_NEXT;
                recipient = nullptr;
                value = 0.0f;
                total = 0;
                return;
            }
        }
    }
};

#endif
