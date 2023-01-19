// Author: Alex Hartford
// Program: Emblem
// File: Animation

#ifndef ANIMATION_H
#define ANIMATION_H

/* From Game Engine Architecture:
   Event Channel
   * Footstep sound
   * Cloud of dust Particle effect
   YOU CAN HOOK THESE THINGS UP TO EVENTS.
   For instance:
   * Unit_Died event results in the dialogue system playing and changing the interface state.
*/

// ================================ Animation ==================================
float
lerp(float a, float b, float amount)
{
    return a * (1.0 - amount) + (b * amount);
}

enum AnimationValue
{
    ATTACK_ANIMATION_HIT,
    ATTACK_ANIMATION_MISS,
    ATTACK_ANIMATION_CRITICAL,
};

struct Sample
{
    float value;
};

struct Channel
{
    Sample samples[30] = {};
    float *value;
};

struct Animation
{
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;
    int finish  = 0;
    bool repeat = false;
    Channel channels[4] = {};
    Event on_finish;

    Animation(int speed_in, int finish_in, bool repeat_in,
              Event on_finish_in)
    : speed(speed_in),
      finish(finish_in),
      repeat(repeat_in),
      on_finish(on_finish_in)
    {}

    Animation(const Animation &other)
    : speed(other.speed),
      finish(other.finish),
      repeat(other.repeat),
      on_finish(other.on_finish)
    {
        // TODO: Make sure this is right.
        for(int i = 0; i < 4; ++i)
        {
            channels[i] = other.channels[i];
        }
    }

    // called each frame
    bool
    Update()
    {
        counter++;
        if(!(counter % finish))
        {
            if(repeat)
            {
                counter = 0;
                return false;
            }

            EmitEvent(on_finish);
            return true;
        }
        return false;
    }
};

Animation *
GetAnimation(AnimationValue anim)
{
    switch(anim)
    {
        case ATTACK_ANIMATION_HIT: return (new Animation(1, 20, false, Event(EVENT_ANIMATION_COMPLETE)));
        case ATTACK_ANIMATION_MISS: return (new Animation(1, 20, false, Event(EVENT_ANIMATION_COMPLETE)));
        case ATTACK_ANIMATION_CRITICAL: return (new Animation(1, 20, false, Event(EVENT_ANIMATION_COMPLETE)));
        default:
        {
            assert(!"ERROR GetAnimation: Animation type not defined\n");
            return nullptr;
        }
    }
}

struct AnimationSystem
{
    Animation *current = nullptr;
    // TODO: Array of animations

    void
    PlayAnimation(AnimationValue in)
    {
        current = GetAnimation(in);
    }

    void
    Update()
    {
        if(current)
        {
            if(current->Update())
            {
                delete(current);
                current = nullptr;
            }
        }
    }
};

static AnimationSystem GlobalAnimations;

#endif
