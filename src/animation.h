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
Identity(float t)
{
    return t;
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
EaseInCubic(float t)
{
    return t * t * t;
}

float
EaseInQuadratic(float t)
{
    return t * t * t * t;
}

float
EaseInQuintic(float t)
{
    return t * t * t * t * t;
}

float
EaseOut(float t)
{
    return Flip(EaseIn(Flip(t)));
}

float
EaseOutCubic(float t)
{
    return Flip(EaseInCubic(Flip(t)));
}

float
EaseOutQuadratic(float t)
{
    return Flip(EaseInQuadratic(Flip(t)));
}

float
EaseOutQuintic(float t)
{
    return Flip(EaseInQuintic(Flip(t)));
}

float
EaseInOut(float t)
{
    return Lerp(EaseIn(t), EaseOut(t), t);
}

float
EaseInOutCustom(float t, float (*in) (float), float (*out) (float))
{
    return Lerp(in(t), out(t), t);
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
    float t;
    float value;
};

struct Channel
{
    vector<Sample> samples = {{0.0, 0.0}, {0.25, 0.5}, {0.75, 0.0}, {1.0, 0.0}};
    int index = 0;
    float (*ease) (float) = Identity;

    void
    Update(float time)
    {
        if(samples[index+1].t < time)
        {
            ++index;
        }
    }

    float
    Value(float time)
    {
        return Lerp(samples[index].value, samples[index+1].value,
                    ease(time));
    }
};

struct Animation
{
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;
    int finish  = 0;
    bool repeat = false;
    Channel channel;

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
        return channel.Value(Time());
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
        counter++;
        if(!(counter % finish))
        {
            if(repeat)
            {
                counter = 0;
                return false;
            }

            //EmitEvent(on_finish);
            return true;
        }
        channel.Update(Time());
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
