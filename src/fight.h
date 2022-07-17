// Author: Alex Hartford
// Program: Emblem
// File: Fight
// Date: July 2022

#ifndef FIGHT_H
#define FIGHT_H

// Rolls a d100. range: 00 to 99.
int d100()
{
    return rand() % 100;
}

// Determines what damage a hit will do.
int CalculateDamage(int attack, int defense)
{
    return max(attack - defense, 0);
}

// Simulates a single combat between a unit and their enemy.
void SimulateCombat(Unit *one, Unit *two, bool oneAttacking, bool twoAttacking)
{
	int damage;
    // one -> two
	if(oneAttacking)
	{
		damage = CalculateDamage(one->attack, two->defense);
		if(d100() < one->accuracy)
		{
			if(two->hp - damage <= 0)
			{
				two->hp = 0;
				two->shouldDie = true;
			}
			else
			{
				two->hp -= damage;
			}
		}
	}

	if(twoAttacking && !two->shouldDie)
	{
		// two -> one
		damage = CalculateDamage(two->attack, one->defense);
		if(d100() > two->accuracy)
		{
			if(one->hp - damage <= 0)
			{
				one->hp = 0;
				one->shouldDie = true;
			}
			else
			{
				one->hp -= damage;
			}
		}
	}
}

void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->healing;
    two->hp = min(healing + two->hp, two->maxHp);
}

#endif
