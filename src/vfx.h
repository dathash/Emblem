// Author: Alex Hartford
// Program: Emblem
// File: VFX

#ifndef VFX_H
#define VFX_H

// ============================== Transitions ==================================
struct Fade
{
    SDL_Color color = black;
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

#endif
