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
SimulateDamage(Unit *victim, int amount, int probability)
{
    if(!victim)
        return;

    if(Roll(d100) > probability)
    {
        cout << "Missed!\n";
        return;
    }

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

    if(!victim) // TODO: for now. Eventually, we want to be able to attack anything.
        return;

    int damage = 1;
    int probability = 100;
    for(const Modifier &mod : source->modifiers)
    {
        switch(mod.type)
        {
            case MOD_RANCOR: damage += mod.tier; break;
            default: cout << "Modifier had no effect\n";
        }
    }
    for(const Modifier &mod : victim->modifiers)
    {
        switch(mod.type)
        {
            case MOD_ARMOR: damage -= mod.tier; break;
            case MOD_DODGE: probability -= (20 * mod.tier); break;
            default: cout << "Modifier had no effect\n";
        }
    }

    SimulateDamage(victim, clamp(damage, 0, 9), clamp(probability, 0, 100));
}

#endif
