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

    void
    Execute()
    {
        // TODO: Different attack types result in different animations
        if(hit && !crit)
        {
            // TODO: How do I decouple this damage from here?
            // I want it to happen when the animation finishes.
            GlobalAnimations.PlayAnimation(ATTACK_ANIMATION_HIT);
            target->Damage(damage);
        }
        else if(hit && crit)
        {
            GlobalAnimations.PlayAnimation(ATTACK_ANIMATION_CRITICAL);
            target->Damage(CRIT_MULTIPLIER * damage);
        }
        else
        {
            GlobalAnimations.PlayAnimation(ATTACK_ANIMATION_MISS);
            return;
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

    queue<Attack> attack_queue = {};

    bool ready = false;

    Fight() = default;

    Fight(Unit *one_in, Unit *two_in, 
          int one_avo_in, int two_avo_in,
          int distance_in)
    : one(one_in),
      two(two_in),
      one_avoid_bonus(one_avo_in),
      two_avoid_bonus(two_avo_in),
      distance(distance_in)
    {
        Populate(PredictCombat(*one_in, *two_in,
                               distance_in,
                               one_avo_in,
                               two_avo_in));
    }

    void
    Update()
    {
        if(ready)
        {
            if(!attack_queue.empty())
            {
                Attack next = attack_queue.front();
                attack_queue.pop();

                next.Execute();
                cout << next << "\n";
            }
            else
            {
                EmitEvent(EVENT_COMBAT_OVER);
            }
            ready = false;
        }
    }

    void
    Populate(const Outcome &outcome)
    {
        int one_dmg = CalculateDamage(*one, *two);
        int two_dmg = CalculateDamage(*two, *one);
        Attack attack = {one, two, one_dmg};
        if(d100() < HitChance(*one, *two, two_avoid_bonus))
        {
            attack.hit = true;
            if(d100() < CritChance(*one, *two))
                attack.crit = true;
        }
        attack_queue.push(attack);

        if(attack.hit && 
           (two->health - attack.damage <= 0 ||
            attack.crit && two->health - attack.damage * CRIT_MULTIPLIER <= 0))
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

            if(attack.hit && 
               (one->health - attack.damage <= 0 ||
                attack.crit && one->health - attack.damage * CRIT_MULTIPLIER <= 0))
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

        if(attack.hit && 
           (two->health - attack.damage <= 0 ||
            attack.crit && two->health - attack.damage * CRIT_MULTIPLIER <= 0))
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

    void
    AnimationComplete()
    {

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
