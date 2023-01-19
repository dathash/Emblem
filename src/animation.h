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
Lerp(float a, float b, float amount)
{
    return a * (1.0 - amount) + (b * amount);
}

float
Flip(float t)
{
    return 1 - t;
}

float
EaseIn(float t)
{
    return t * t;
}

float
EaseOut(float t)
{
    return Flip(EaseIn(Flip(t)));
}

float
EaseInOut(float t)
{
    return Lerp(EaseIn(t), EaseOut(t), t);
}

float
Spike(float t)
{
    if (t <= .5f)
        return EaseIn(t/0.5);
 
    return EaseIn(Flip(t)/0.5);
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
    float t;
};

struct Animation
{
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;
    int finish  = 0;
    bool repeat = false;
    vector<Sample> channel = {{0.0, 0.0}, {0.5, 0.5}, {0.0, 1.0}};
    int channel_index = 0;

    Animation() = default;

    Animation(int speed_in, int finish_in, bool repeat_in)
    : speed(speed_in),
      finish(finish_in),
      repeat(repeat_in)
    {}

    Animation(const Animation &other)
    : speed(other.speed),
      finish(other.finish),
      repeat(other.repeat)
    {
        channel = other.channel;
    }

    float
    Value()
    {
        return Lerp(channel[channel_index].value, channel[channel_index+1].value,
                    EaseOut(Time()));
    }

    float
    Time()
    {
        return (float)counter / finish;
    }

    // called each frame
    bool
    Update()
    {
        //cout << channel_index << "\n";
        counter++;
        if(channel[channel_index+1].t < Time())
        {
            ++channel_index;
        }

        if(!(counter % finish))
        {
            if(repeat)
            {
                counter = 0;
                channel_index = 0;
                return false;
            }

            //EmitEvent(on_finish);
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
        case ATTACK_ANIMATION_HIT: return (new Animation(1, 40, false));
        case ATTACK_ANIMATION_MISS: return (new Animation(1, 40, false));
        case ATTACK_ANIMATION_CRITICAL: return (new Animation(1, 40, false));
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
