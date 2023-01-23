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
    int attack = predator.attack;
    int defense = prey.defense;
    if(predator.buff && predator.buff->stat == STAT_ATTACK)
        attack += predator.buff->amount;
    if(prey.buff && prey.buff->stat == STAT_DEFENSE)
        defense += prey.buff->amount;
    return clamp(attack - defense, 0, 999);
}

// Determines the speed difference between two units.
bool
Doubles(const Unit &predator, const Unit &prey)
{
    int pred_spd = predator.speed;
    int prey_spd = prey.speed;
    if(predator.buff && predator.buff->stat == STAT_SPEED)
        pred_spd += predator.buff->amount;
    if(prey.buff && prey.buff->stat == STAT_SPEED)
        prey_spd += prey.buff->amount;
    return pred_spd - prey_spd > DOUBLE_RATIO;
}

int
CalculateHealing(const Unit &healer, const Unit &healee)
{
    int healing = healer.aptitude;
    if(healer.buff && healer.buff->stat == STAT_SPEED)
        healing += healer.buff->amount;
    return healing;
}

// Declares the outcome of a coming altercation.
// For information passing in combat functions.
struct Outcome
{
    bool one_attacks;
    bool one_doubles;
    int one_damage;
    int one_hit;
    int one_crit;
    bool two_attacks;
    bool two_doubles;
    int two_damage;
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
    outcome.one_attacks = true;
    outcome.one_damage = CalculateDamage(one, two);
    outcome.one_hit = HitChance(one, two, two_avoid_bonus);
    outcome.one_crit = one.crit;
    outcome.one_doubles = Doubles(one, two);

    if(distance >= two.min_range && distance <= two.max_range)
    {
        outcome.two_attacks = true;
        outcome.two_damage = CalculateDamage(two, one);
        outcome.two_hit = HitChance(two, one, one_avoid_bonus);
        outcome.two_crit = two.crit;
        outcome.two_doubles = Doubles(two, one);
    }

    return outcome;
}

enum AttackType
{
    MELEE,
    RANGED,
    //MAGIC,
    //DIVINE,
};

struct Attack
{
    Unit *source;
    Unit *target;
    int damage;

    bool hit;
    bool crit;
    AttackType type;

    // Starts up animations.
    Animation *
    Execute()
    {
        Animation *animation;
        switch (type)
        {
            case RANGED:
            {
                animation = GetAnimation(ATTACK_ANIMATION_RANGED);
            } break;

            case MELEE:
            {
                if(hit && !crit)
                {
                    animation = GetAnimation(ATTACK_ANIMATION_MELEE);
                    EmitEvent(ATTACK_HIT);
                }
                else if(hit && crit)
                {
                    animation = GetAnimation(ATTACK_ANIMATION_LEAP);
                    EmitEvent(ATTACK_CRIT);
                }
                else
                {
                    animation = GetAnimation(ATTACK_ANIMATION_MISS);
                    EmitEvent(ATTACK_MISS);
                }
            } break;
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
              << ", crit: " << a.crit
              << ", type: " << a.type;
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
    bool lower_half_screen = false;

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
        // TODO: Move animations out of this class into a global animation manager
        if(animation)
        {
            direction dir = one_to_two_direction;
            if(Current().source == two)
                dir = dir * -1;
            float value = animation->Value(CHANNEL_ONE);
            position offset = (dir * TILE_SIZE) * value;

            float value_two = animation->Value(CHANNEL_TWO);
            offset += (direction(0, -1) * TILE_SIZE) * value_two;
            Current().source->animation_offset = offset;

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
                //cout << attack_queue.front() << "\n";
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
        lower_half_screen = false;
        quadrant quad = Quadrant(one->pos);
        if(quad == BOTTOM_LEFT
           || quad == BOTTOM_RIGHT)
            lower_half_screen = true;
            int one_dmg = CalculateDamage(*one, *two);
        int two_dmg = CalculateDamage(*two, *one);
        int one_accum = 0;
        int two_accum = 0;
        Attack attack = {one, two, one_dmg};
        if(distance > 1)
        {
            attack.type = RANGED;
        }
        if(d100() < HitChance(*one, *two, two_avoid_bonus))
        {
            attack.hit = true;
            if(d100() < CritChance(*one, *two))
                attack.crit = true;
        }
        attack_queue.push(attack);

        if(attack.hit)
            two_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

        if(attack.hit && two->health - two_accum <= 0)
            return;

        if(distance >= two->min_range && distance <= two->max_range)
        {
            attack = {two, one, two_dmg};
            if(distance > 1)
            {
                attack.type = RANGED;
            }
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
        if(distance > 1)
        {
            attack.type = RANGED;
        }
        if(outcome.one_doubles)
        {
            if(d100() < HitChance(*one, *two, two_avoid_bonus))
            {
                attack.hit = true;
                if(d100() < CritChance(*one, *two))
                    attack.crit = true;
            }
            attack_queue.push(attack);
            if(attack.hit)
                two_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

            if(attack.hit && two->health - two_accum <= 0)
                return;
        }

        if(distance >= two->min_range && distance <= two->max_range)
        {
            attack = {two, one, two_dmg};
            if(distance > 1)
            {
                attack.type = RANGED;
            }
            if(outcome.two_doubles)
            {
                if(d100() < HitChance(*two, *one, one_avoid_bonus))
                {
                    attack.hit = true;
                    if(d100() < CritChance(*two, *one))
                        attack.crit = true;
                }
                attack_queue.push(attack);
            } 
        }
    }
};

// =============================== Healing =====================================
// Displays the outcome of one unit healing another.
Outcome PredictHealing(const Unit &one, const Unit &two)
{
    Outcome outcome = {};
    outcome.one_attacks = true;
    outcome.one_doubles = false;
    outcome.one_damage = CalculateHealing(one, two);
    outcome.one_hit = 100;
    outcome.one_crit = 0;

    return outcome;
}


// Simulates one unit healing another.
void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->aptitude;
    two->health = min(healing + two->health, two->max_health);
}

// Simulates one unit healing another.
void SimulateDancing(Unit *one, Unit *two)
{
    // one -> two
    two->Activate();
}

#endif
