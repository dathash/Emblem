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

    return {-1, -1};
}

void
SimulateDamage(Unit *victim, int amount)
{
    if(!victim)
        return;

    if(victim->ID() == hash<string>{}("House"))
    {
        GlobalPlayer.Damage(amount);
    }

    // NOTE: This is called when damage is zero when pushing a unit.
    // Keep that in mind for visualization purposes.
    cout << amount << " damage to " << victim->name << "\n";

    victim->Damage(amount);
    if(victim->health <= 0)
        victim->should_die = true;
}

void
SimulateHeal(Unit *victim, int amount)
{
    if(!victim)
        return;

    if(victim->ID() == hash<string>{}("House"))
    {
        GlobalPlayer.Heal(amount);
    }

    cout << amount << " healing to " << victim->name << "\n";

    victim->Heal(amount);
}

void
SimulatePush(Tilemap *map, int push_damage,
             const position &initial, const direction &dir)
{
    cout << "Pushing " << initial << " towards " << dir << "\n";
    if(!IsValid(initial)) return;
    Unit *pushed = map->tiles[initial.col][initial.row].occupant;
    if(!pushed) return;

    // Damage occupants (innate push damage on the weapon)
    cout << pushed->name << " pushed.\n";
    SimulateDamage(pushed, push_damage);

    if(pushed->fixed)
        return;

    // Do the pushing
    position target = initial + dir;
    if(!IsValid(target)) return;

    Unit *bonk = map->tiles[target.col][target.row].occupant;

    if(bonk)
    {
        cout << "collision with " << bonk->name << ".\n";
        SimulateDamage(map->tiles[initial.col][initial.row].occupant, 1);
        SimulateDamage(map->tiles[target.col][target.row].occupant, 1);
    }
    else
    {
        cout << "No Collision.\n";
        map->tiles[target.col][target.row].occupant = map->tiles[initial.col][initial.row].occupant;
        map->tiles[initial.col][initial.row].occupant = nullptr;
        map->tiles[target.col][target.row].occupant->pos = target;
    }
}

void
PerformPushScenario(Tilemap *map, PushType type, int push_damage,
                    const position &pos, const direction &dir)
{
    cout << "PUSH SCENARIO BEGINS. " << pos << " with direction " << dir << "\n";
    switch(type)
    {
        case PUSH_NONE:
        {

        } break;
        case PUSH_AWAY:
        {
            SimulatePush(map, push_damage, pos, dir);
        } break;
        case PUSH_TOWARDS:
        {
            SimulatePush(map, push_damage, pos, dir * -1);
        } break;
        case PUSH_TOWARDS_AND_AWAY:
        {
            SimulatePush(map, push_damage, pos + dir, dir);
            SimulatePush(map, push_damage, pos - dir, dir * -1);
        } break;
        case PUSH_PERPENDICULAR:
        {
            direction perp = {dir.row, dir.col};
            SimulatePush(map, push_damage, pos + perp, perp);
            SimulatePush(map, push_damage, pos - perp, perp * -1);
        } break;
        case PUSH_ALL:
        {
            SimulatePush(map, push_damage, pos + dir, dir);
            SimulatePush(map, push_damage, pos - dir, dir * -1);

            direction perp = {dir.row, dir.col};
            SimulatePush(map, push_damage, pos + perp, perp);
            SimulatePush(map, push_damage, pos - perp, perp * -1);
        } break;
    }
}

void
SimulateMove(Tilemap *map, int self_damage,
             const position &source, const position &target)
{
    cout << "Moving " << source << " to " << target << "\n";
    if(!IsValid(target) || !IsValid(source) || (source == target)) return;
    cout << "Moved.\n";

    assert(map->tiles[source.col][source.row].occupant); // pretty sure
    SimulateDamage(map->tiles[source.col][source.row].occupant, self_damage);

    // Move
    if(map->tiles[target.col][target.row].occupant)
    {
        // Bonk!
        SimulateDamage(map->tiles[source.col][source.row].occupant, 1);
        SimulateDamage(map->tiles[target.col][target.row].occupant, 1);
    }
    else if(map->tiles[source.col][source.row].occupant)
    {
        // Push 'em!
        map->tiles[target.col][target.row].occupant = map->tiles[source.col][source.row].occupant;
        map->tiles[source.col][source.row].occupant = nullptr;
        map->tiles[target.col][target.row].occupant->pos = target;
    }
}

void
PerformMoveScenario(Tilemap *map, MovementType type, int self_damage,
                    const position &source, const position &target,
                    const direction &dir)
{
    cout << "MOVE SCENARIO BEGINS\n";
    switch(type)
    {
        case MOVEMENT_NONE:
        {

        } break;
        case MOVEMENT_BACKONE:
        {
            SimulateMove(map, self_damage, source, source - dir);
        } break;
        case MOVEMENT_RAM:
        {
            if(map->tiles[target.col][target.row].occupant)
            {
                cout << target << "|||" << dir << "|||" << source << "\n";
                SimulateMove(map, self_damage, source, target - dir);
            }
            else
                SimulateMove(map, self_damage, source, target);
        } break;
        case MOVEMENT_LEAP:
        {
            SimulateMove(map, self_damage, source, target);
        } break;
    }
}

void
Simulate(Tilemap *map,
         const Equip &weapon, 
         position source, 
         position destination)
{
    switch(weapon.type)
    {
    case EQUIP_NONE:
    {
        cout << "No Weapon type: " << weapon.type << "\n";
    } break;
    case EQUIP_PUNCH:
    {
        position subject = destination;

        direction with = Normalized((direction)(destination - source));
        vector<Unit *> victims = {};
        for(int i = 1; i <= weapon.max_range; ++i)
        {
            position target = source + (with * i);
            if(!IsValid(target)) continue;
            Unit *victim = map->tiles[target.col][target.row].occupant;
            if(victim) victims.push_back(victim);
        }

        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));
        PerformPushScenario(map, weapon.push, weapon.push_damage, 
                            subject, GetDirection(source, subject));

        cout << victims.size() << "\n";
        for(Unit *victim : victims)
            SimulateDamage(victim, weapon.damage);
    } break;
    case EQUIP_LINE_SHOT:
    {
        position subject = GetFirstTarget(*map, source, 
                                          GetDirection(source, destination));
        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));

        if(!IsValid(subject)) return;

        PerformPushScenario(map, weapon.push, weapon.push_damage, 
                            subject, GetDirection(source, subject));
        SimulateDamage(victim, weapon.damage);

    } break;
    case EQUIP_ARTILLERY:
    {
        position subject = destination;

        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));
        PerformPushScenario(map, weapon.push, weapon.push_damage, 
                            subject, GetDirection(source, subject));
        SimulateDamage(victim, weapon.damage);
    } break;
    case EQUIP_SELF_TARGET:
    {
        cout << "Unimplemented weapon type: " << weapon.type << "\n";
    } break;
    case EQUIP_LEAP:
    {
        position subject = destination;

        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));
        PerformPushScenario(map, weapon.push, weapon.push_damage, 
                            subject, GetDirection(source, subject));
        cout << "Damaging\n";
        SimulateDamage(victim, weapon.damage);
    } break;
    case EQUIP_LASER:
    {
        cout << "Unimplemented weapon type: " << weapon.type << "\n";
    } break;
    case EQUIP_HEAL:
    {
        position subject = destination;

        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        SimulateHeal(victim, weapon.damage);
    } break;
    }
}

#endif
