// Author: Alex Hartford
// Program: Emblem
// File: Fight
// Date: July 2022

#ifndef FIGHT_H
#define FIGHT_H

void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->healing;
    two->hp = min(healing + two->hp, two->maxHp);
}

#endif
