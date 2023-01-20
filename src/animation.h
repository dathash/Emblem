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
EaseIn2(float t)
{
    return t * t;
}

float
EaseIn3(float t)
{
    return t * t * t;
}

float
EaseIn4(float t)
{
    return t * t * t * t;
}

float
EaseIn5(float t)
{
    return t * t * t * t * t;
}

float
EaseOut2(float t)
{
    return Flip(EaseIn2(Flip(t)));
}

float
EaseOut3(float t)
{
    return Flip(EaseIn3(Flip(t)));
}

float
EaseOut4(float t)
{
    return Flip(EaseIn4(Flip(t)));
}

float
EaseOut5(float t)
{
    return Flip(EaseIn5(Flip(t)));
}

float
EaseInOut(float t)
{
    return Lerp(EaseIn2(t), EaseOut2(t), t);
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
        return EaseIn2(t/0.5);
 
    return EaseIn2(Flip(t)/0.5);
}

enum AnimationValue
{
    ATTACK_ANIMATION_HIT,
    ATTACK_ANIMATION_MISS,
    ATTACK_ANIMATION_CRITICAL,
};

enum ChannelIndex
{
    CHANNEL_ONE,
    CHANNEL_TWO,
    CHANNEL_THREE,
    CHANNEL_FOUR,
};

struct Sample
{
    float t;
    float value;
};

struct Channel
{
    vector<Sample> samples = {};
    int index = 0;
    float (*ease) (float) = Identity;

    Channel() = default;

    Channel(const vector<Sample> &samples_in,
            float (*ease_in) (float))
    : samples(samples_in),
      ease(ease_in)
    {}

    void
    Update(float time)
    {
        assert(!samples.empty());
        if(samples[index+1].t < time)
        {
            ++index;
        }
    }

    float
    Value(float time)
    {
        float ratio = (ease(time) - samples[index].t) / (samples[index+1].t - samples[index].t);
        return Lerp(samples[index].value, samples[index+1].value,
                    ratio);
    }
};

struct Animation
{
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;
    int finish  = 0;
    bool repeat = false;
    Channel channel_one   = {};
    Channel channel_two   = {};
    Channel channel_three = {};
    Channel channel_four  = {};
    int num_channels = 0;

    Animation() = default;

    Animation(int speed_in, int finish_in, bool repeat_in, int num_channels_in,
              const vector<Sample> &samples_one, float (*ease_one) (float),
              const vector<Sample> &samples_two = {}, float (*ease_two) (float) = Identity,
              const vector<Sample> &samples_three = {}, float (*ease_three) (float) = Identity,
              const vector<Sample> &samples_four = {}, float (*ease_four) (float) = Identity
              )
    : speed(speed_in),
      finish(finish_in),
      repeat(repeat_in),
      num_channels(num_channels_in)
    {
        channel_one = Channel(samples_one, ease_one);
        channel_two = Channel(samples_two, ease_two);
        channel_three = Channel(samples_three, ease_three);
        channel_four = Channel(samples_four, ease_four);
    }

    Animation(const Animation &other)
    : speed(other.speed),
      finish(other.finish),
      repeat(other.repeat),
      num_channels(other.num_channels)
    {
        // TODO: This is still suspect
        channel_one = other.channel_one;
        channel_two = other.channel_two;
        channel_three = other.channel_three;
        channel_four = other.channel_four;
    }

    float
    Value(ChannelIndex index)
    {
        switch (index)
        {
            case CHANNEL_ONE: return channel_one.Value(Time());
            case CHANNEL_TWO: return channel_two.Value(Time());
            case CHANNEL_THREE: return channel_three.Value(Time());
            case CHANNEL_FOUR: return channel_four.Value(Time());
            default: assert(!"We only support four channels of animation.\n"); return 0.0f;
        }
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
        if(num_channels > 0)
            channel_one.Update(Time());
        if(num_channels > 1)
            channel_two.Update(Time());
        if(num_channels > 2)
            channel_three.Update(Time());
        if(num_channels > 3)
            channel_four.Update(Time());
        return false;
    }
};

Animation *
GetAnimation(AnimationValue anim)
{
    switch(anim)
    {
        case ATTACK_ANIMATION_HIT:
        {
            return (new Animation(1, 40, false, 2,
                    {{0.0 ,  0.0 },  // channel 1
                     {0.4 ,  0.0 },
                     {0.5 ,  1.0 },
                     {1.0 ,  0.0 }},
                     Identity,
                    {{0.0 ,  0.0 },  // channel 2
                     {0.40,  0.0 },
                     {0.45,  0.5 },
                     {0.5 ,  0.0 },
                     {1.0 ,  0.0 }},
                     Identity));
        }
        case ATTACK_ANIMATION_MISS: 
        {
            return (new Animation(1, 40, false, 2,
                    {{0.0 ,  0.0 },  // channel 1
                     {0.4 ,  0.0 },
                     {0.5 ,  1.0 },
                     {1.0 ,  0.0 }},
                     Identity,
                    {{0.0 ,  0.0 },  // channel 2
                     {0.40,  0.0 },
                     {0.45,  0.5 },
                     {0.5 ,  0.0 },
                     {1.0 ,  0.0 }},
                     Identity));
        }
        case ATTACK_ANIMATION_CRITICAL: 
        {
            return (new Animation(1, 40, false, 2,
                    {{0.0 ,  0.0 },  // channel 1
                     {0.4 ,  0.0 },
                     {0.5 ,  1.0 },
                     {1.0 ,  0.0 }},
                     Identity,
                    {{0.0 ,  0.0 },  // channel 2
                     {0.40,  0.0 },
                     {0.45,  0.5 },
                     {0.5 ,  0.0 },
                     {1.0 ,  0.0 }},
                     Identity));
        }
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
