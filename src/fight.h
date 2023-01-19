// Author: Alex Hartford
// Program: Emblem
// File: Fight

#ifndef FIGHT_H
#define FIGHT_H

// Rolls a d100. range: 00 to 99.
int
d100()
{
    return rand() % 100;
}

// Returns the chance to hit a unit
int
HitChance(const Unit &predator, const Unit &prey, int bonus)
{
    return (predator.accuracy - prey.avoid - bonus);
}

// Returns the chance to crit a unit
// CONSIDER: critical resist mechanic, like hobbit luck?
int
CritChance(const Unit &predator, const Unit &prey)
{
    return (predator.crit);
}

// Determines what damage a hit will do.
int
CalculateDamage(const Unit &predator, const Unit &prey)
{
    return clamp(predator.attack - prey.defense, 0, 999);
}

int
CalculateHealing(const Unit &healer, const Unit &healee)
{
    return healer.ability;
}

// Declares the outcome of a coming altercation.
// For information passing in combat functions.
struct Outcome
{
    bool one_double;
    int one_attack;
    int one_hit;
    int one_crit;
    bool two_double;
    int two_attack;
    int two_hit;
    int two_crit;
};

// =============================== Attacking =====================================
// Makes a worst-case combat prediction for:
// UI display when a player initiates combat
// AI Decision-making and target-selection
// Returns the struct defined above.
Outcome
PredictCombat(const Unit &one, const Unit &two, int distance,
			  int one_avoid_bonus, int two_avoid_bonus)
{
    Outcome outcome = {};
    outcome.one_attack = CalculateDamage(one, two);
    outcome.one_hit = HitChance(one, two, two_avoid_bonus);
    outcome.one_crit = one.crit;
    outcome.one_double = one.speed - two.speed > DOUBLE_RATIO;

    if(distance >= two.min_range && distance <= two.max_range)
    {
        outcome.two_attack = CalculateDamage(two, one);
        outcome.two_hit = HitChance(two, one, one_avoid_bonus);
        outcome.two_crit = two.crit;
        outcome.two_double = two.speed - one.speed > DOUBLE_RATIO;
    }

    return outcome;
}

/*
enum AttackType
{
    MELEE,
    RANGED,
};
*/

struct Attack
{
    Unit *source;
    Unit *target;
    int damage;

    bool hit;
    bool crit;
    // AttackType type;

    // Starts up animations.
    Animation *
    Execute()
    {
        Animation *animation;
        // TODO: Different attack types result in different animations
        if(hit && !crit)
        {
            animation = GetAnimation(ATTACK_ANIMATION_HIT);
        }
        else if(hit && crit)
        {
            animation = GetAnimation(ATTACK_ANIMATION_CRITICAL);
        }
        else
        {
            animation = GetAnimation(ATTACK_ANIMATION_MISS);
        }
        return animation;
    }

    // Actually Applies damage
    void
    Resolve()
    {
        if(hit && !crit)
        {
            target->Damage(damage);
        }
        else if(hit && crit)
        {
            target->Damage(damage * CRIT_MULTIPLIER);
        }
    }
};
std::ostream
&operator<<(std::ostream &os, const Attack &a)
{
    return os << "Attack | " << a.source->name << " on " << a.target->name 
              << " | dmg: " << a.damage 
              << ", hit: " << a.hit 
              << ", crit: " << a.crit;
}


struct Fight
{
    Unit *one = nullptr;
    Unit *two = nullptr;
    int one_avoid_bonus = 0;
    int two_avoid_bonus = 0;
    int distance = 0;
    direction one_to_two_direction = {0, 0};

    queue<Attack> attack_queue = {};

    bool ready = false;

    Animation *animation = nullptr;

    Fight() = default;

    Fight(Unit *one_in, Unit *two_in, 
          int one_avo_in, int two_avo_in,
          int distance_in, const direction &direction_in)
    : one(one_in),
      two(two_in),
      one_avoid_bonus(one_avo_in),
      two_avoid_bonus(two_avo_in),
      distance(distance_in)
    {
        cout << direction_in << "\n";
        one_to_two_direction = direction_in;
        Populate(PredictCombat(*one_in, *two_in,
                               distance_in,
                               one_avo_in,
                               two_avo_in));
        ready = true;
    }

    Attack
    Current() const
    {
        return attack_queue.front();
    }

    void
    Update()
    {
        if(animation)
        {
            direction dir = one_to_two_direction;
            if(Current().source == two)
                dir = dir * -1;
            float value = animation->Value();
            Current().source->animation_offset = (dir * TILE_SIZE) * value;
            if(animation->Update())
            {
                Current().Resolve();
                attack_queue.pop();
                ready = true;
                delete animation;
                animation = nullptr;
            }
        }
        if(ready)
        {
            if(!attack_queue.empty())
            {
                animation = Current().Execute();
                cout << attack_queue.front() << "\n";
            }
            else
            {
                // TODO: Put an extra animation in
                // here which waits to kill units
                // until the fight is resolved.
                if(one->health <= 0)
                    one->should_die = true;
                if(two->health <= 0)
                    two->should_die = true;
                if(GlobalPlayerTurn)
                {
                    one->Deactivate();

                    GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                    if(one->should_die)
                    {
                        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
                    }
                }
                else
                {
                    one->Deactivate();
                    GlobalAIState = FINDING_NEXT;
                }

                //EmitEvent(EVENT_COMBAT_OVER);
            }
            ready = false;
        }
    }

    void
    Populate(const Outcome &outcome)
    {
        int one_dmg = CalculateDamage(*one, *two);
        int two_dmg = CalculateDamage(*two, *one);
        int one_accum = 0;
        int two_accum = 0;
        Attack attack = {one, two, one_dmg};
        if(d100() < HitChance(*one, *two, two_avoid_bonus))
        {
            attack.hit = true;
            if(d100() < CritChance(*one, *two))
                attack.crit = true;
        }
        attack_queue.push(attack);

        if(attack.hit)
            two_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

        cout << two_accum << "\n";

        if(attack.hit && two->health - two_accum <= 0)
            return;

        if(distance >= two->min_range && distance <= two->max_range)
        {
            attack = {two, one, two_dmg};
            if(d100() < HitChance(*two, *one, one_avoid_bonus))
            {
                attack.hit = true;
                if(d100() < CritChance(*two, *one))
                    attack.crit = true;
            }
            attack_queue.push(attack);

            if(attack.hit)
                one_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

            if(attack.hit && one->health - one_accum <= 0)
                return;
        }

        attack = {one, two, one_dmg};
        if(one->speed - two->speed > DOUBLE_RATIO)
        {
            if(d100() < HitChance(*one, *two, two_avoid_bonus))
            {
                attack.hit = true;
                if(d100() < CritChance(*one, *two))
                    attack.crit = true;
            }
        }
        attack_queue.push(attack);

        if(attack.hit)
            two_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

        if(attack.hit && two->health - two_accum <= 0)
            return;

        if(distance >= two->min_range && distance <= two->max_range)
        {
            attack = {two, one, two_dmg};
            if(two->speed - one->speed > DOUBLE_RATIO)
            {
                if(d100() < HitChance(*two, *one, one_avoid_bonus))
                {
                    attack.hit = true;
                    if(d100() < CritChance(*two, *one))
                        attack.crit = true;
                }
            } 
            attack_queue.push(attack);
        }
    }
};

// =============================== Healing =====================================
// Displays the outcome of one unit healing another.
Outcome PredictHealing(const Unit &one, const Unit &two)
{
    Outcome outcome = {};
    outcome.one_attack = CalculateHealing(one, two);
    outcome.one_hit = 100;
    outcome.one_crit = 0;

    return outcome;
}


// Simulates one unit healing another.
void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->ability;
    two->health = min(healing + two->health, two->max_health);
}


#endif
