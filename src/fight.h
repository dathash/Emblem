// Author: Alex Hartford
// Program: Emblem
// File: Fight
// Date: July 2022

#ifndef FIGHT_H
#define FIGHT_H

int d100()
{
    return rand() % 100;
}

void SimulateCombat(Unit *one, Unit *two)
{
    // one -> two
    int damage = one->attack - two->defense;
    if(d100() < one->accuracy)
    {
        printf("SIMULATION | %d Damage!\n", damage);
        if(two->hp - damage <= 0)
        {
            printf("SIMULATION | Target Died!\n");
            two->hp = 0;
            two->shouldDie = true;
        }
        else
        {
            two->hp -= damage;
        }
    }
    else
    {
        printf("SIMULATION | Missed!\n");
    }

    // two -> one
    damage = two->attack - one->defense;
    if(d100() > two->accuracy)
    {
        printf("SIMULATION | %d Damage!\n", damage);
        if(one->hp - damage <= 0)
        {
            printf("SIMULATION | Unit Died!\n");
            one->hp = 0;
            one->shouldDie = true;
        }
        else
        {
            one->hp -= damage;
        }
    }
    else
    {
        printf("SIMULATION | Missed!\n");
    }
}

void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->healing;
    printf("SIMULATION | %d Healing!\n", healing);
    two->hp = min(healing + two->hp, two->maxHp);
}

#endif
