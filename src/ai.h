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
        selected = FindNearest(point(cursor->col, cursor->row), map,
                [](const Unit &unit) -> bool
                {
                    return !unit.isAlly && !unit.isExhausted;
                });

        if(selected)
        {
            // move cursor
            cursor->col = selected->col;
            cursor->row = selected->row;

            // For rendering
            MoveViewport(cursor->col, cursor->row);
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
        cursor->selected = map->tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;

        map->accessible = AccessibleFrom(*map, cursor->selectedCol,
                                         cursor->selectedRow,
                                         cursor->selected->mov,
                                         cursor->selected->isAlly);

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
        cursor->selected->isExhausted = true;

        cursor->selected->sheet.ChangeTrack(0);
        cursor->selected = nullptr;

        cursor->selectedCol = -1;
        cursor->selectedRow = -1;

        GlobalAIState = FINDING_NEXT;
    }
private:
    Cursor *cursor; 
};

// Scans the map and determines the best course of action to take.
// Uses techniques specified by the unit's ai_behavior field.
pair<point, Unit *>
GetAction(const Unit &unit, const Tilemap &map)
{
    pair<point, Unit *> action = {};
    vector<pair<point, Unit *>> possibilities = FindAttackingSquares(map, unit);

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
                Unit *nearest = FindNearest(point(unit.col, unit.row), map,
                    [](const Unit &unit) -> bool
                    {
                        return unit.isAlly;
                    });
                path path_to_nearest = GetPath(map, unit.col, unit.row, nearest->col, nearest->row, false);
                action = pair<point, Unit *>(
                        FurthestMovementOnPath(map, path_to_nearest, unit.mov),
                                             NULL);
            }
            else
            {
                int min_health_after_attack = 999;
                Outcome outcome;
                //int max_odds = 0;
                //int min_counter_dmg = 100;
                //int min_counter_odds = 100;
                for(const pair<point, Unit *> &poss : possibilities)
                {
                    const point &p = poss.first;
                    Unit *t = poss.second;
                    outcome = PredictCombat(unit, *t,
                                            ManhattanDistance(p, point(t->col, t->row)),
                                            map.tiles[p.first][p.second].avoid,
                                            map.tiles[t->col][t->row].avoid);
                    if(outcome.two_health < min_health_after_attack)
                    {
                        action = poss;
                        min_health_after_attack = outcome.two_health;
                    }
                }
            }
        } break;
        case BOLSTER:
        {
            if(possibilities.size() == 0) // No enemies to attack in range.
            {
                action = {point(unit.col, unit.row),
                          NULL};
            }
            else
            {
                int min_health_after_attack = 999;
                Outcome outcome;

                action = {point(unit.col, unit.row),
                          NULL};
                for(const pair<point, Unit *> &poss : possibilities)
                {
                    if(poss.first.first == unit.col && poss.first.second == unit.row)
                    {
                        const point &p = poss.first;
                        Unit *t = poss.second;
                        outcome = PredictCombat(unit, *t,
                                                ManhattanDistance(p, point(t->col, t->row)),
                                                map.tiles[p.first][p.second].avoid,
                                                map.tiles[t->col][t->row].avoid);
                        if(outcome.two_health < min_health_after_attack)
                        {
                            action = poss;
                            min_health_after_attack = outcome.two_health;
                        }
                    }
                }
            }
        } break;
        case FLEE:
        {
            action = {point(unit.col, unit.row),
                      NULL};
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
    AIPerformUnitActionCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        // Find target
        pair<point, Unit *> action = GetAction(*cursor->selected, *map);
        assert(!(action.first == point(0, 0)));

        // move cursor
        cursor->col = action.first.first;
        cursor->row = action.first.second;
        MoveViewport(cursor->col, cursor->row);

        // place unit
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;

        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->sourceCol = cursor->col;
        cursor->sourceRow = cursor->row;

        cursor->selected->sheet.ChangeTrack(1);

        // perform attack
        if(action.second)
        {
            int distance = ManhattanDistance(point(cursor->selected->col, cursor->selected->row),
                                             point(action.second->col, action.second->row));
            SimulateCombat(cursor->selected, action.second, distance,
                           map->tiles[cursor->selectedCol][cursor->selectedRow].avoid,
                           map->tiles[cursor->col][cursor->row].avoid);
        }

        // resolution
        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        // change state
        GlobalAIState = FINDING_NEXT;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

// ============================== struct ====================================
struct AI
{
    int frame = 0;

    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Tilemap *map)
    {
        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, *map));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, map));
    }

    // Passes the args through to plan.
    void Update(Cursor *cursor, Tilemap *map)
    {
        if(GlobalPlayerTurn)
            return;

        if(commandQueue.empty())
            Plan(cursor, map);

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
