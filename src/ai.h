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
                    return !unit.IsAlly() && !unit.is_exhausted;
                }, false);

        if(selected)
        {
            // move cursor
            cursor->pos = selected->pos;
        }
        else
        {
            GlobalPlayerTurn = true;
            level->turn_start = true;
            EmitEvent(END_TURN_EVENT);
            GlobalInterfaceState = NO_OP;
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
        cursor->selected->initial_pos = cursor->pos;

        map->accessible.clear();
        pair<vector<position>, vector<position>> result = 
            AccessibleAndAttackableFrom(*map, cursor->pos,
                                        cursor->selected->movement,
                                        1,
                                        1,
                                        cursor->selected->IsAlly());
        map->accessible = result.first;

        GlobalAIState = AI_SELECTED;
    }
private:
    Cursor *cursor;
    Tilemap *map;
};


pair<position, Unit *>
AttackInRangeBehavior(const Unit &unit, const Tilemap &map)
{
    pair<position, Unit *> action = {};
    vector<pair<position, Unit *>> possibilities = FindAttackingSquares(map, unit, map.accessible);
    return possibilities[0];
}


// Scans the map and determines the best course of action to take.
// Uses techniques specified by the unit's ai_behavior field.
pair<position, Unit *>
GetAction(const Unit &unit, const Tilemap &map)
{
    return AttackInRangeBehavior(unit, map);    
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
        pair<position, Unit *> action = GetAction(*cursor->selected, *map);
        SDL_assert(!(action.first == position(0, 0)));

        // move cursor
        cursor->pos = action.first;

        // place unit
        map->tiles[cursor->selected->initial_pos.col][cursor->selected->initial_pos.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(TRACK_ACTIVE);

        // perform attack
        if(action.second)
        {
            cursor->pos = action.second->pos;

            GlobalAIState = AI_FINDING_NEXT;
            return;
        }

        // resolution
        cursor->pos = cursor->selected->pos;
        cursor->selected->Deactivate();
        cursor->selected = nullptr;

        // change state
        GlobalAIState = AI_FINDING_NEXT;
        return;
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
    void Plan(Cursor *cursor, Level *level)
    {
        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, level));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, &(level->map)));
        commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, &(level->map)));
    }

    // Passes the args through to plan.
    void Update(Cursor *cursor, Level *level)
    {
        if(GlobalPlayerTurn)
            return;

        if(GlobalAIState == AI_NO_OP ||
           GlobalAIState == AI_ATTACK_RESOLUTION
          )
            return;

        if(commandQueue.empty())
        {
            Plan(cursor, level);
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
