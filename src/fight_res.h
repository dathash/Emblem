// Author: Alex Hartford
// Program: Emblem
// File: Fight Resolver
// Date: November 2022

// NOTE: This is a temporary solution until I get some more
//       ideas down about how animations are supposed to
//       work in a fucking video game.

#ifndef FIGHT_RES_H
#define FIGHT_RES_H

struct CombatResolver
{
    Unit *attacker;
    Unit *victim;
    bool counterAttack;
    int framesActive;
    int inc;
	int damageToAttacker;
	int damageToVictim;

    CombatResolver()
    {}

    void Update()
    {
        if(inc == 0)
        {
			SimulateCombat();
            printf("%p, %p, %d\n", attacker, victim, counterAttack);
        }
		printf("%d\n", inc);
        ++inc;
        if(inc > framesActive)
        {
			WrapUp();
        }
    }

    void WrapUp()
    {
        AssignDamage();
		attacker->isExhausted = true;
		attacker->sheet.ChangeTrack(0);

		// Reset Fields
        attacker = NULL;
        victim = NULL;
        counterAttack = false;
        inc = 0;
		damageToAttacker = 0;
		damageToVictim = 0;

		// Transition back to normal play
		if(GlobalPlayerTurn)
		{
			GlobalResolvingAction = false;
			GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
			printf("Transitioning | player's turn!\n");
		}
		else
		{
			GlobalResolvingAction = false;
			GlobalAIState = FINDING_NEXT;
			printf("Transitioning | enemy's turn!\n");
		}
    }

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

    // Simulates a single combat between a unit and their enemy
    // NOTE: Sets fields in this very struct that will be
    //       applied to characters after animation.
    void
	SimulateCombat()
    {
		printf("Simulating Combat...\n");
        if(d100() < attacker->accuracy)
        {
			damageToVictim = CalculateDamage(attacker->attack, victim->defense);
			printf("%d\n", damageToVictim);
			if(damageToVictim > victim->hp)
			{
				counterAttack = false;
			}
        }

        if(counterAttack)
        {
            if(d100() > victim->accuracy)
            {
				printf("%d\n", damageToAttacker);
				damageToAttacker = CalculateDamage(victim->attack, attacker->defense);
            }
        }
    }

	void
	AssignDamage()
	{
		printf("Assigning Damage!\n");
		if(victim->hp - damageToVictim <= 0)
		{
			victim->hp = 0;
		}
		else
		{
			victim->hp -= damageToVictim;
		}

		if(attacker->hp - damageToAttacker <= 0)
		{
			attacker->hp = 0;
		}
		else
		{
			attacker->hp -= damageToAttacker;
		}
	}
};

#endif
