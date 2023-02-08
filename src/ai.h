// Author: Alex Hartford
// Program: Emblem
// File: AI

#ifndef AI_H
#define AI_H

// ============================= ai commands ================================
class AIFindNextUnitCommand : public Command
{
public:
    AIFindNextUnitCommand(Cursor *cursor_in, Level *level_in)
    : cursor(cursor_in),
      level(level_in)
    {}

    virtual void Execute()
    {
        // find a unit that hasn't acted yet.
        Unit *selected = nullptr;
        selected = FindNearest(level->map, cursor->pos,
                [](const Unit &unit) -> bool
                {
                    return !unit.is_ally && !unit.is_exhausted;
                }, false);

        if(selected)
        {
            // move cursor
            cursor->pos = selected->pos;
        }
        else
        {
            GlobalAIState = PLAYER_TURN;
            GlobalPlayerTurn = true;
            level->turn_start = true;
            EmitEvent(END_AI_TURN_EVENT);
        }
    }
private: 
    Cursor *cursor;
    Level *level;
};


class AISelectUnitCommand : public Command
{
public:
    AISelectUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;
        cursor->redo = cursor->pos;

        map->accessible.clear();
        map->vis_range.clear();
        pair<vector<position>, vector<position>> result = 
            AccessibleAndAttackableFrom(*map, cursor->redo,
                                        cursor->selected->movement,
                                        cursor->selected->min_range,
                                        cursor->selected->max_range,
                                        cursor->selected->is_ally);
        map->accessible = result.first;
        map->vis_range = result.second;

        map->double_range = 
            AccessibleAndAttackableFrom(*map, cursor->redo,
                                        cursor->selected->movement * 2,
                                        cursor->selected->min_range,
                                        cursor->selected->max_range,
                                        cursor->selected->is_ally).first;

        GlobalAIState = SELECTED;
    }
private:
    Cursor *cursor;
    Tilemap *map;
};


class AIDeactivateUnitCommand : public Command
{
public:
    AIDeactivateUnitCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->selected->is_exhausted = true;

        cursor->selected->sheet.ChangeTrack(TRACK_IDLE);
        cursor->selected = nullptr;

        cursor->redo = {-1, -1};

        GlobalAIState = FINDING_NEXT;
    }
private:
    Cursor *cursor; 
};

// =============================== Specification of Behaviors ==================
pair<position, Unit *>
PursueBehavior(const Unit &unit, const Tilemap &map)
{
    pair<position, Unit *> action = {};
    vector<pair<position, Unit *>> possibilities = FindAttackingSquares(map, unit, map.accessible);
    if(possibilities.size() == 0) // No enemies to attack in range.
    {
        Unit *nearest = FindNearest(map, unit.pos,
            [](const Unit &unit) -> bool
            {
                return unit.is_ally;
            }, false);
        path path_to_nearest = GetPath(map, unit.pos, nearest->pos, false);
        if(path_to_nearest.size())
        {
            position furthest = FurthestMovementOnPath(map, path_to_nearest, unit.movement);
            if(furthest == position(0, 0))
            {
                furthest = unit.pos;
            }
            action = pair<position, Unit *>(furthest, NULL);
        }
        else
            action = {unit.pos, NULL};
    }
    else
    {
        int min_health_after_attack = 999;
        Outcome outcome;
        //int max_odds = 0;
        //int min_counter_dmg = 100;
        //int min_counter_odds = 100;
        for(const pair<position, Unit *> &poss : possibilities)
        {
            const position &p = poss.first;
            Unit *target = poss.second;
            outcome = PredictCombat(unit, *target,
                                    ManhattanDistance(p, target->pos),
                                    map.tiles[p.col][p.row].avoid,
                                    map.tiles[target->pos.col][target->pos.row].avoid,
                                    map.tiles[p.col][p.row].defense,
                                    map.tiles[target->pos.col][target->pos.row].defense);
            int health_remaining = clamp(target->health - outcome.two_damage * (1 + outcome.two_doubles), 0, target->health);
            if(health_remaining < min_health_after_attack)
            {
                action = poss;
                min_health_after_attack = health_remaining;
            }
        }
    }
    return action;
}


pair<position, Unit *>
BossBehavior(const Unit &unit, const Tilemap &map)
{
    pair<position, Unit *> action = {};
    vector<pair<position, Unit *>> possibilities = FindAttackingSquares(map, unit, map.accessible);

    if(possibilities.size() == 0) // No enemies to attack in range.
    {
        action = {unit.pos, NULL};
    }
    else
    {
        int min_health_after_attack = 999;
        Outcome outcome;

        action = {unit.pos, NULL};
        for(const pair<position, Unit *> &poss : possibilities)
        {
            if(poss.first == unit.pos)
            {
                const position &p = poss.first;
                Unit *target = poss.second;
                outcome = PredictCombat(unit, *target,
                                        ManhattanDistance(p, target->pos),
                                        map.tiles[p.col][p.row].avoid,
                                        map.tiles[target->pos.col][target->pos.row].avoid,
                                        map.tiles[p.col][p.row].defense,
                                        map.tiles[target->pos.col][target->pos.row].defense);
                int health_remaining = clamp(target->health - outcome.two_damage * (1 + outcome.two_doubles), 0, target->health);
                if(health_remaining < min_health_after_attack)
                {
                    action = poss;
                    min_health_after_attack = health_remaining;
                }
            }
        }
    }
    return action;
}


pair<position, Unit *>
AttackInRangeBehavior(const Unit &unit, const Tilemap &map, bool extended)
{
    pair<position, Unit *> action = {};
    vector<pair<position, Unit *>> possibilities = FindAttackingSquares(map, unit, map.accessible);
    vector<pair<position, Unit *>> extended_poss;
    if(extended)
         extended_poss = FindAttackingSquares(map, unit, map.double_range);

    if(possibilities.empty()) // No enemies to attack in range.
    {
        if(extended_poss.empty())
            action = {unit.pos, NULL};
        else
        {
            Unit *nearest = FindNearest(map, unit.pos,
                [](const Unit &unit) -> bool
                {
                    return unit.is_ally;
                }, false);
            path path_to_nearest = GetPath(map, unit.pos, nearest->pos, false);
            if(path_to_nearest.size())
            {
                position furthest = FurthestMovementOnPath(map, path_to_nearest, unit.movement);
                if(furthest == position(0, 0))
                {
                    furthest = unit.pos;
                }
                action = pair<position, Unit *>(furthest, NULL);
            }
        }
    }
    else
    {
        int min_health_after_attack = 999;
        Outcome outcome;

        action = {unit.pos, NULL};
        for(const pair<position, Unit *> &poss : possibilities)
        {
            const position &p = poss.first;
            Unit *target = poss.second;
            outcome = PredictCombat(unit, *target,
                                    ManhattanDistance(p, target->pos),
                                    map.tiles[p.col][p.row].avoid,
                                    map.tiles[target->pos.col][target->pos.row].avoid,
                                    map.tiles[p.col][p.row].defense,
                                    map.tiles[target->pos.col][target->pos.row].defense);
            int health_remaining = clamp(target->health - outcome.two_damage * (1 + outcome.two_doubles), 0, target->health);
            if(health_remaining < min_health_after_attack)
            {
                action = poss;
                min_health_after_attack = health_remaining;
            }
        }
    }
    return action;
}


// Scans the map and determines the best course of action to take.
// Uses techniques specified by the unit's ai_behavior field.
pair<position, Unit *>
GetAction(const Unit &unit, const Tilemap &map)
{
    switch(unit.ai_behavior)
    {
        case PURSUE:             return PursueBehavior(unit, map);
        case PURSUE_AFTER_1:     return ((unit.turns_active >= 1) ? PursueBehavior(unit, map) : AttackInRangeBehavior(unit, map, false));
        case PURSUE_AFTER_2:     return ((unit.turns_active >= 2) ? PursueBehavior(unit, map) : AttackInRangeBehavior(unit, map, false));
        case PURSUE_AFTER_3:     return ((unit.turns_active >= 3) ? PursueBehavior(unit, map) : AttackInRangeBehavior(unit, map, false));
        case BOSS:               return BossBehavior(unit, map);
        case BOSS_THEN_MOVE:     return ((unit.health == unit.max_health) ? BossBehavior(unit, map) : PursueBehavior(unit, map));
        case ATTACK_IN_RANGE:    return AttackInRangeBehavior(unit, map, false);
        case ATTACK_IN_TWO:      return AttackInRangeBehavior(unit, map, true);
        case FLEE:               return {unit.pos, NULL};
        case TREASURE_THEN_FLEE: return {unit.pos, NULL};
        case NO_BEHAVIOR: cout << "WARN AIPerformUnitActionCommand: This AI Unit has no behavior specified.\n"; return {};
        default: SDL_assert(!"Shouldn't get here!\n"); return {};
    }
}

class AIPerformUnitActionCommand : public Command
{
public:
    AIPerformUnitActionCommand(Cursor *cursor_in, Tilemap *map_in,
                               Fight *fight_in)
    : cursor(cursor_in),
      map(map_in),
      fight(fight_in)
    {}

    virtual void Execute()
    {
        // Find target
        pair<position, Unit *> action = GetAction(*cursor->selected, *map);
        SDL_assert(!(action.first == position(0, 0)));

        // move cursor
        cursor->pos = action.first;

        // place unit
        map->tiles[cursor->redo.col][cursor->redo.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->source = cursor->pos;
        cursor->selected->sheet.ChangeTrack(TRACK_ACTIVE);

        // perform attack
        if(action.second)
        {
            int distance = ManhattanDistance(cursor->selected->pos,
                                             action.second->pos);
            direction dir = GetDirection(cursor->selected->pos,
                                         action.second->pos);
            *fight = Fight(cursor->selected, action.second,
                          map->tiles[cursor->redo.col][cursor->redo.row].avoid,
                          map->tiles[cursor->pos.col][cursor->pos.row].avoid,
                          map->tiles[cursor->redo.col][cursor->redo.row].defense,
                          map->tiles[cursor->pos.col][cursor->pos.row].defense,
                          distance, dir);
            fight->ready = true;

            // resolution
            cursor->selected = nullptr;
            cursor->targeted = nullptr;
            cursor->pos = cursor->source;

            GlobalAIState = AI_FIGHT;
            return;
        }

        // resolution
        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->pos = cursor->source;

        // change state
        GlobalAIState = FINDING_NEXT;
        return;
    }

private:
    Cursor *cursor;
    Tilemap *map;
    Fight *fight;
};

// ============================== struct ====================================
struct AI
{
    int frame = 0;

    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Level *level, Fight *fight)
    {
        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, level));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, &(level->map)));
        commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, &(level->map), fight));
    }

    // Passes the args through to plan.
    void Update(Cursor *cursor, Level *level, Fight *fight)
    {
        if(GlobalPlayerTurn)
            return;

        if(GlobalAIState == AI_FIGHT ||
           GlobalAIState == AI_RESOLVING_EXPERIENCE) // TODO: Simplify these states. Reduce bugs.
            return;

        if(commandQueue.empty())
        {
            Plan(cursor, level, fight);
            // TODO: Bug with Experience Parceling
        }
        

        ++frame;
        // Every __ frames.
        if(!(frame % AI_ACTION_SPEED))
        {
            commandQueue.front()->Execute();
            commandQueue.pop();
        }
    }

    void clearQueue()
    {
        commandQueue = {};
    }

private:
    queue<shared_ptr<Command>> commandQueue;
};

#endif
