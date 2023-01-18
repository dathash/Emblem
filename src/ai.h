// Author: Alex Hartford
// Program: Emblem
// File: AI

#ifndef AI_H
#define AI_H

// ============================= ai commands ================================
class AIFindNextUnitCommand : public Command
{
public:
    AIFindNextUnitCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        // find a unit that hasn't acted yet.
        Unit *selected = nullptr;
        selected = FindNearest(map, cursor->pos,
                [](const Unit &unit) -> bool
                {
                    return !unit.is_ally && !unit.is_exhausted;
                }, false);

        if(selected)
        {
            // move cursor
            cursor->pos = selected->pos;

            // For rendering
            SetViewport(cursor->pos, map.width, map.height);
        }
        else
        {
            EndAITurn();
        }
    }
private: 
    Cursor *cursor;
    const Tilemap &map;
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

        map->accessible = AccessibleFrom(*map, cursor->redo,
                                         cursor->selected->movement,
                                         cursor->selected->is_ally);

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

        cursor->selected->sheet.ChangeTrack(0);
        cursor->selected = nullptr;

        cursor->redo = {-1, -1};

        GlobalAIState = FINDING_NEXT;
    }
private:
    Cursor *cursor; 
};

// Scans the map and determines the best course of action to take.
// Uses techniques specified by the unit's ai_behavior field.
pair<position, Unit *>
GetAction(const Unit &unit, const Tilemap &map)
{
    pair<position, Unit *> action = {};
    vector<pair<position, Unit *>> possibilities = FindAttackingSquares(map, unit);

    switch(unit.ai_behavior)
    {
        case NO_BEHAVIOR:
        {
            cout << "WARN AIPerformUnitActionCommand: This AI Unit has no behavior specified.\n";
        } break;
        case PURSUE:
        {
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
                                            map.tiles[target->pos.col][target->pos.row].avoid);
                    int health_remaining = clamp(target->health - outcome.two_attack * (1 + outcome.two_double), 0, target->health);
                    if(health_remaining < min_health_after_attack)
                    {
                        action = poss;
                        min_health_after_attack = health_remaining;
                    }
                }
            }
        } break;
        case BOLSTER:
        {
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
                                                map.tiles[target->pos.col][target->pos.row].avoid);
                        int health_remaining = clamp(target->health - outcome.two_attack * (1 + outcome.two_double), 0, target->health);
                        if(health_remaining < min_health_after_attack)
                        {
                            action = poss;
                            min_health_after_attack = health_remaining;
                        }
                    }
                }
            }
        } break;
        case WAIT_THEN_ATTACK:
        {
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
                    const position &p = poss.first;
                    Unit *target = poss.second;
                    outcome = PredictCombat(unit, *target,
                                            ManhattanDistance(p, target->pos),
                                            map.tiles[p.col][p.row].avoid,
                                            map.tiles[target->pos.col][target->pos.row].avoid);
                    int health_remaining = clamp(target->health - outcome.two_attack * (1 + outcome.two_double), 0, target->health);
                    if(health_remaining < min_health_after_attack)
                    {
                        action = poss;
                        min_health_after_attack = health_remaining;
                    }
                }
            }
        } break;
        case FLEE:
        {
            action = {unit.pos, NULL};
        } break;

        default:
        {
            assert(!"Shouldn't get here!\n");
        } break;
    }
    return action;
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
        assert(!(action.first == position(0, 0)));

        // move cursor
        cursor->pos = action.first;
        SetViewport(cursor->pos, map->width, map->height);

        // place unit
        map->tiles[cursor->redo.col][cursor->redo.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->source = cursor->pos;
        cursor->selected->sheet.ChangeTrack(1);

        // perform attack
        if(action.second)
        {
            int distance = ManhattanDistance(cursor->selected->pos,
                                             action.second->pos);
            *fight = Fight(cursor->selected, action.second,
                          map->tiles[cursor->redo.col][cursor->redo.row].avoid,
                          map->tiles[cursor->pos.col][cursor->pos.row].avoid,
                          distance);
        }

        // resolution
        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->pos = cursor->source;

        // change state
        GlobalAIState = FINDING_NEXT;
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
    void Plan(Cursor *cursor, Tilemap *map, Fight *fight)
    {
        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, *map));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, map, fight));
    }

    // Passes the args through to plan.
    void Update(Cursor *cursor, Tilemap *map, Fight *fight)
    {
        if(GlobalPlayerTurn)
            return;

        if(commandQueue.empty())
            Plan(cursor, map, fight);

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
