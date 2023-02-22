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

#endif
