// Author: Alex Hartford
// Program: Emblem
// File: Fight

#ifndef FIGHT_H
#define FIGHT_H

// For use with line_shot mechs.
position
GetFirstTarget(const Tilemap &map, 
               const position &pos, const direction &dir, 
               int min_range, int max_range)
{
    vector<position> line = Line(map, pos, dir);
    for(int i = min_range-1; i <= min(max_range, line.size()); ++i)
    {
        if(map.tiles[line[i].col][line[i].row].occupant)
            return line[i];
    }

    cout << "WARNING GetFirstTarget(): No targets.\n";
    return {-1, -1};
}

void
SimulateDamage(Unit *victim, int amount)
{
    if(!victim)
        return;

    // NOTE: This is called when damage is zero when pushing a unit.
    // Keep that in mind for visualization purposes.
    //cout << amount << " Damage to " << victim->name << "\n";
    victim->Damage(amount);
    if(victim->health <= 0)
        victim->should_die = true;
}

void
SimulatePush(Tilemap *map, int push_damage,
             const position &initial, const direction &dir)
{
    position target = initial + dir;
    if(!IsValid(target) || !IsValid(initial))
        return;

    if(!map->tiles[initial.col][initial.row].occupant)
        return;
        
    SimulateDamage(map->tiles[initial.col][initial.row].occupant, push_damage);

    if(map->tiles[target.col][target.row].occupant)
    {
        // Bonk!
        SimulateDamage(map->tiles[initial.col][initial.row].occupant, 1);
        SimulateDamage(map->tiles[target.col][target.row].occupant, 1);
    }
    else if(map->tiles[initial.col][initial.row].occupant)
    {
        // Push 'em!
        map->tiles[target.col][target.row].occupant = map->tiles[initial.col][initial.row].occupant;
        map->tiles[initial.col][initial.row].occupant = nullptr;
        map->tiles[target.col][target.row].occupant->pos = target;
    }
}

void
PerformPushScenario(Tilemap *map, PushType type, int push_damage,
                    const position &pos, const direction &dir)
{
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
        default:
        {
        } break;
    }
}

void
SimulateMove(Tilemap *map, int self_damage,
             const position &source, const position &target)
{
    if(!IsValid(target) || !IsValid(source))
        return;

    if(map->tiles[source.col][source.row].occupant)
        SimulateDamage(map->tiles[source.col][source.row].occupant, self_damage);

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
            SimulateMove(map, self_damage, source, target - dir);
        } break;
        case MOVEMENT_LEAP:
        {
            SimulateMove(map, self_damage, source, target);
        } break;
        default:
        {
        } break;
    }
}

void
Simulate(Tilemap *map, const Equip &weapon, 
         const position &source, 
         const position &destination)
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
        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));
        PerformPushScenario(map, weapon.push, weapon.push_damage, 
                            subject, GetDirection(source, subject));
        SimulateDamage(victim, weapon.damage);
    } break;
    case EQUIP_LINE_SHOT:
    {
        position subject = GetFirstTarget(*map, source, 
                                         GetDirection(source, destination),
                                         weapon.min_range, weapon.max_range);
        if(!IsValid(subject))
            return;

        Unit *victim = map->tiles[subject.col][subject.row].occupant;
        PerformMoveScenario(map, weapon.move, weapon.self_damage, 
                            source, subject, GetDirection(source, subject));
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
        SimulateDamage(victim, weapon.damage);
    } break;
    case EQUIP_LASER:
    {
        cout << "Unimplemented weapon type: " << weapon.type << "\n";
    } break;
    default:
    {
        cout << "Unimplemented weapon type: " << weapon.type << "\n";
    } break;
    }
}

#endif
