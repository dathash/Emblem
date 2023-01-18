// Author: Alex Hartford
// Program: Emblem
// File: Animation

#ifndef ANIMATION_H
#define ANIMATION_H

/* From Game Engine Architecture:
   Channels
   Metachannels
   Event Channel
   * Footstep sound
   * Cloud of dust Particle effect
   YOU CAN HOOK THESE THINGS UP TO EVENTS.
   For instance:
   * Unit_Died event results in the dialogue system playing and changing the interface state.
*/

// ================================ Animation ==================================
enum AnimationValue
{
    ATTACK_ANIMATION_HIT,
    ATTACK_ANIMATION_MISS,
    ATTACK_ANIMATION_CRITICAL,
};

struct Animation
{
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;
    int frames  = 0;
    bool repeat = false;
    Event on_finish;

    Animation(int speed_in, int frames_in, bool repeat_in,
              Event on_finish_in)
    : speed(speed_in),
      frames(frames_in),
      repeat(repeat_in),
      on_finish(on_finish_in)
    {}

    Animation(const Animation &other)
    : speed(other.speed),
      frames(other.frames),
      repeat(other.repeat),
      on_finish(other.on_finish)
    {}

    // called each frame
    bool
    Update()
    {
        counter++;
        cout << counter << "\n";
        if(!(counter % frames))
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
