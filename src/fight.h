// Author: Alex Hartford
// Program: Emblem
// File: Fight

#ifndef FIGHT_H
#define FIGHT_H

// For use with line_shot mechs.
position
GetFirstTarget(const Tilemap &map, 
               const position &pos, const direction &dir)
{
    vector<position> line = Line(map, pos, dir);
    for(int i = 0; i < line.size(); ++i)
    {
        if(map.tiles[line[i].col][line[i].row].occupant)
            return line[i];
    }

    if(!line.empty())
        return line.back();

    //cout << "WARNING GetFirstTarget(): No targets.\n";
    return {-1, -1};
}

void
SimulateDamage(Unit *victim, int amount)
{
    if(!victim)
        return;

    //cout << amount << " Damage to " << victim->name << "\n";
    victim->Damage(amount);
    if(victim->health <= 0)
        victim->should_die = true;
}

void
Simulate(Tilemap *map, 
         Unit *source,
         const position &destination)
{
    Unit *victim = map->tiles[destination.col][destination.row].occupant;
    SimulateDamage(victim, 1);
}

// ======================================== Resolution System ==================

struct Attack
{
    Unit *unit;
    position offset;

    void
    Resolve(Tilemap *map)
    {
        cout << "RESOLUTION\n";
        cout << unit->name << "\n";
        cout << unit->pos << "\n";
        cout << offset << "\n";
        Simulate(map, unit, unit->pos + offset);
    }
};

struct Resolution
{
    vector<Attack> attacks;
    int frame = 0;

    void
    Update(Tilemap *map)
    {
        if(attacks.empty())
            GoToAIPhase();

        ++frame;
        if(frame % AI_ACTION_SPEED)
            return;

        attacks.back().Resolve(map);
        attacks.pop_back();
    }

    void
    RemoveDeadUnits()
    {
        attacks.erase(remove_if(attacks.begin(), attacks.end(),
                    [](auto const &a) { return a.unit->should_die; }),
                    attacks.end());
    }
};

#endif
