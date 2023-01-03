// Author: Alex Hartford
// Program: Emblem
// File: Fight
// Date: January 2023

#ifndef FIGHT_H
#define FIGHT_H

// NOTE: This is a non-animation situation. I want to write the animation code
// separately, so that it doesn't become such a spaghetti mess. We'll see how
// that goes.

// Below is a better system, which should be implemented when animation work begins.
/*
struct Attack
{
    int damage;
    bool hit;
    bool crit;
};

struct Combat
{
    Unit *one;
    Unit *two;
    // NOTE: Fields are either an attack or null.
    Attack first;
    Attack second;
    Attack third;
    Attack fourth;

    // Populates a Combat struct with attacks based on battle logic.
    void
    Populate()
    {
    }
};

*/

// Rolls a d100. range: 00 to 99.
int d100()
{
    return rand() % 100;
}

// Returns the chance to hit a unit
int HitChance(const Unit &predator, const Unit &prey)
{
    return (predator.accuracy - prey.avoid);
}

// Determines what damage a hit will do.
int CalculateDamage(const Unit &predator, const Unit &prey)
{
    return clamp(predator.attack - prey.defense, 0, 100); // TODO: maximum damage???
}

// Simulates one unit healing another.
void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->healing;
    two->hp = min(healing + two->hp, two->maxHp);
}

// For return in function below.
struct Outcome
{
    int one_hit;
    int one_crit;
    int one_health;
    int two_hit;
    int two_crit;
    int two_health;
};

// Makes a worst-case combat prediction for the sake of UI display
// when a player initiates combat.
Outcome
PredictCombat(const Unit &one, const Unit &two, int distance)
{
    Outcome outcome = {0, 0, one.hp, 0, 0, two.hp};
    outcome.one_hit = HitChance(one, two);
    outcome.one_crit = one.crit;
    outcome.two_health = clamp(two.hp - CalculateDamage(one, two), 0, two.maxHp);

    if(distance >= two.minRange && distance <= two.maxRange)
    {
        outcome.two_hit = HitChance(two, one);
        outcome.two_crit = two.crit;
        outcome.one_health = clamp(one.hp - CalculateDamage(two, one), 0, one.maxHp);
    }
    // TODO: Check if Fire Emblem doesn't display a possibility of
    // enemy attack if you are guaranteed to kill them.

    return outcome;
}

// Simulates a single combat between a unit and their enemy.
// Includes logic for different distances.
void
SimulateCombat(Unit *one, Unit *two, int distance)
{
    int one_dmg = CalculateDamage(*one, *two);
    int two_dmg = CalculateDamage(*two, *one);
    if(d100() < HitChance (*one, *two))
    {
        two->Damage(one_dmg);
    }

    if(two->hp > 0 && d100() < HitChance(*two, *one) &&
       (distance >= two->minRange && distance <= two->maxRange))
    {
        one->Damage(two_dmg);
    }

    one->isExhausted = true;
    one->sheet.ChangeTrack(0);
}

#endif
