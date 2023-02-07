// Author: Alex Hartford
// Program: Emblem
// File: Fight

#ifndef FIGHT_H
#define FIGHT_H

// Returns the chance to hit a unit
int
HitChance(const Unit &predator, const Unit &prey, int bonus)
{
    int hit = predator.Hit();

    int avoid = prey.Avoid() + bonus;

    return (hit - avoid - bonus);
}

// Returns the chance to crit a unit
// CONSIDER: critical resist mechanic, like hobbit luck?
int
CritChance(const Unit &predator, const Unit &prey)
{
    return (predator.Crit());
}

// Determines what damage a hit will do.
int
CalculateDamage(const Unit &predator, const Unit &prey, int defense_bonus)
{
    int attack = predator.Attack();

    int defense = prey.defense + defense_bonus;
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
    int pred_spd = predator.AttackSpeed();
    int prey_spd = prey.AttackSpeed();
    if(predator.buff && predator.buff->stat == STAT_SPEED)
        pred_spd += predator.buff->amount;
    if(prey.buff && prey.buff->stat == STAT_SPEED)
        prey_spd += prey.buff->amount;
    return pred_spd - prey_spd > DOUBLE_RATIO;
}

int
CalculateHealing(const Unit &healer, const Unit &healee)
{
    int healing = healer.magic;
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
			  int one_avoid_bonus, int two_avoid_bonus,
			  int one_defense_bonus, int two_defense_bonus
              )
{
    Outcome outcome = {};
    outcome.one_attacks = true;
    outcome.one_damage = CalculateDamage(one, two, two_defense_bonus);
    outcome.one_hit = HitChance(one, two, two_avoid_bonus);
    outcome.one_crit = one.Crit();
    outcome.one_doubles = Doubles(one, two);

    if(distance >= two.MinRange() && distance <= two.MaxRange())
    {
        outcome.two_attacks = true;
        outcome.two_damage = CalculateDamage(two, one, one_defense_bonus);
        outcome.two_hit = HitChance(two, one, one_avoid_bonus);
        outcome.two_crit = two.Crit();
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
                animation = GetAnimation(ATTACK_RANGED_ANIMATION, 1.0f);
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
                        EmitEvent(Event(EXPERIENCE_EVENT, one, experience_amount));
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

                    if(two->should_die)
                    {
                        // TODO: Put DEATH CONVERSATION HERE!!!
                        GlobalAIState = FINDING_NEXT;
                    }
                    else
                    {
                        one->Deactivate();
                        EmitEvent(Event(EXPERIENCE_EVENT, two, experience_amount));
                        GlobalAIState = AI_RESOLVING_EXPERIENCE;
                    }
                }
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
            int one_dmg = CalculateDamage(*one, *two, two_defense_bonus);
        int two_dmg = CalculateDamage(*two, *one, one_defense_bonus);
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

        if(distance >= two->MinRange() && distance <= two->MaxRange())
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

        if(distance >= two->MinRange() && distance <= two->MaxRange())
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
    outcome.one_damage = -CalculateHealing(one, two);
    outcome.one_hit = 100;
    outcome.one_crit = 0;

    return outcome;
}

void SimulateBuff(Unit *one, Unit *two)
{
    two->ApplyBuff(new Buff(STAT_ATTACK, 10, 1));
}

void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->magic;
    two->health = min(healing + two->health, two->max_health);
}

void SimulateDancing(Unit *one, Unit *two)
{
    // one -> two
    two->Activate();
}

#endif
