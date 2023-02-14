// Author: Alex Hartford
// Program: Emblem
// File: Fight

#ifndef FIGHT_H
#define FIGHT_H

// Returns a number representing the number required to roll to hit.
int
TargetNumber(const Unit &predator, const Unit &prey, int bonus)
{
    return prey.AC() - predator.ToHit() - bonus;
}

// Returns the chance to crit a unit
// CONSIDER: critical resist mechanic, like hobbit luck?
int
CritNumber(const Unit &predator, const Unit &prey)
{
    return predator.Crit(); // Most always just a 20.
}

// Determines what damage a hit will do.
int
CalculateDamage(const Unit &predator, const Unit &prey, int defense_bonus)
{
    int attack = predator.DamageAmount();
    int defense = 0 + defense_bonus;
    return clamp(attack - defense, 0, 999);
}

int
CalculateHealing(const Unit &healer, const Unit &healee)
{
    int healing = healer.intuition;
    return healing;
}

// Declares the outcome of a coming altercation.
// For information passing in combat functions.
struct Outcome
{
    int (*die)() = d0;
    int num_dice = 0;
    int bonus_damage = 0;
    int target = 0;
    int crit = 0;
};

// =============================== Attacking =====================================
// Makes a worst-case combat prediction for:
// UI display when a player initiates combat
// AI Decision-making and target-selection
// Returns the struct defined above.
Outcome
PredictCombat(const Unit &one, const Unit &two, int distance,
			  int one_avoid_bonus, int two_avoid_bonus,
			  int one_defense_bonus, int two_defense_bonus
              )
{
    Outcome outcome = {};
    outcome.die = d1;
    outcome.num_dice = 1;
    outcome.bonus_damage = one.strength;
    if(one.weapon)
    {
        outcome.die = one.weapon->weapon->die;
        outcome.num_dice = one.weapon->weapon->num_dice;
        outcome.bonus_damage = one.GetWeaponDmgStat();
    }
    outcome.target = TargetNumber(one, two, two_avoid_bonus);
    outcome.crit = one.Crit();

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
                if(hit)
                    animation = GetAnimation(ATTACK_RANGED_HIT_ANIMATION, 1.0f);
                else
                    animation = GetAnimation(ATTACK_RANGED_MISS_ANIMATION, 1.0f);
            } break;

            case MELEE:
            {
                if(hit && !crit)
                {
                    animation = GetAnimation(ATTACK_MELEE_ANIMATION, 1.0f);
                }
                else if(hit && crit)
                {
                    animation = GetAnimation(ATTACK_LEAP_ANIMATION, 1.0f);
                }
                else
                {
                    animation = GetAnimation(ATTACK_MISS_ANIMATION, 1.0f);
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
    int one_defense_bonus = 0;
    int two_defense_bonus = 0;
    int distance = 0;
    direction one_to_two_direction = {0, 0};

    queue<Attack> attack_queue = {};

    bool ready = false;

    Animation *animation = nullptr;
    Animation *finish_animation = nullptr;
    bool lower_half_screen = false;

    Fight() = default;

    Fight(Unit *one_in, Unit *two_in, 
          int one_avo_in, int two_avo_in,
          int one_def_in, int two_def_in,
          int distance_in, const direction &direction_in)
    : one(one_in),
      two(two_in),
      one_avoid_bonus(one_avo_in),
      two_avoid_bonus(two_avo_in),
      one_defense_bonus(one_def_in),
      two_defense_bonus(two_def_in),
      distance(distance_in)
    {
        one_to_two_direction = direction_in;
        Populate(PredictCombat(*one_in, *two_in,
                               distance_in,
                               one_avo_in,
                               two_avo_in,
                               one_def_in,
                               two_def_in));
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
                delete animation;
                animation = nullptr;
                if(attack_queue.empty())
                {
                    finish_animation = GetAnimation(WASTE_TIME_ANIMATION);
                }
                else
                {
                    ready = true;
                }
            }
        }
        if(finish_animation)
        {
            if(finish_animation->Update())
            {
                delete finish_animation;
                finish_animation = nullptr;
                ready = true;
            }
        }
        if(ready)
        {
            ready = false;

            if(!attack_queue.empty())
            {
                animation = Current().Execute();
                //cout << attack_queue.front() << "\n";
            }
            else
            {
                CompleteCombat();
            }
        }
    }


    // Finishes the given combat, changing the game state and reverting the fight.
    // IMPURE
    void
    CompleteCombat()
    {
        if(one->health <= 0)
            one->should_die = true;
        if(two->health <= 0)
            two->should_die = true;

        Unit *experience_recipient = nullptr;
        int experience_amount = 0;

        if(GlobalPlayerTurn)
        {
            experience_recipient = one;

            experience_amount = EXP_FOR_COMBAT;
            if(two->should_die)
                experience_amount += two->xp_value;

            if(one->level > two->level)
                experience_amount /= 2;

            if(one->should_die)
            {
                // TODO: Put DEATH CONVERSATION HERE!!!
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
            }
            else
            {
                one->Deactivate();
                EmitEvent(Event(EXPERIENCE_EVENT, one, experience_amount, (float)experience_amount / 10.0f + 0.3f));
                GlobalInterfaceState = RESOLVING_EXPERIENCE;
            }
        }
        else
        {
            experience_recipient = two;

            experience_amount = 1;
            if(one->should_die)
                experience_amount += one->xp_value;

            if(two->level > one->level + 3) // Arbitrary threshold
                experience_amount /= 4;
            else if(two->level > one->level)
                experience_amount /= 2;

            one->Deactivate();
            if(two->should_die)
            {
                GlobalAIState = AI_FINDING_NEXT;
            }
            else
            {
                EmitEvent(Event(EXPERIENCE_EVENT, two, experience_amount, (float)experience_amount / 10.0f + 0.3f));
                GlobalAIState = AI_RESOLVING_EXPERIENCE;
            }
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
            int one_dmg = CalculateDamage(*one, *two, two_defense_bonus);
        int two_dmg = CalculateDamage(*two, *one, one_defense_bonus);
        int one_accum = 0;
        int two_accum = 0;
        Attack attack = {one, two, one_dmg};
        if(distance > 1)
        {
            attack.type = RANGED;
        }

        int result = Roll(d20);
        if(result >= TargetNumber(*one, *two, two_avoid_bonus))
        {
            attack.hit = true;
            if(result >= CritNumber(*one, *two))
                attack.crit = true;
        }
        attack_queue.push(attack);

        if(attack.hit)
            two_accum += (attack.crit ? attack.damage * CRIT_MULTIPLIER : attack.damage);

        if(attack.hit && two->health - two_accum <= 0)
            return;
    }
};

// =============================== Healing =====================================
// Displays the outcome of one unit healing another.
Outcome PredictHealing(const Unit &one, const Unit &two)
{
    Outcome outcome = {};
    outcome.die = d0;
    outcome.num_dice = 0;
    outcome.bonus_damage = 0;
    if(one.weapon)
    {
        outcome.die = one.weapon->weapon->die;
        outcome.num_dice = one.weapon->weapon->num_dice;
        outcome.bonus_damage = one.GetWeaponDmgStat();
    }
    outcome.target = 0;
    outcome.crit = 20;

    return outcome;
}

bool
SimulateHealing(Unit *one, Unit *two)
{
    int healing = one->intuition;
    two->Heal(healing);
    return true;
}

bool
SimulateGrappling(Unit *one, Unit *two)
{
    int target = max(two->strength, two->dexterity);
    if(Roll(d20) > target)
    {
        two->condition = GetCondition(CONDITION_GRAPPLED);
        return true;
    }
    return false;
}

bool
SimulateDancing(Unit *one, Unit *two)
{
    two->Activate();
    return true;
}

#endif
