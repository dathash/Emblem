// Author: Alex Hartford
// Program: Emblem
// File: AI
// Date: July 2022

#ifndef AI_H
#define AI_H

void EndAITurn()
{
    GlobalAIState = ENEMY_TURN;
    GlobalPlayerTurn = true;
    GlobalTurnStart = true;
}

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
        selected = FindNearest(*cursor, map,
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

        map->accessible.clear();
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


class AIMoveToClosestSquareCommand : public Command
{
public:
    AIMoveToClosestSquareCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        Unit *target = FindNearest(*cursor, *map, 
                [](const Unit &unit) -> bool
                {
                    return unit.isAlly;
                });
		pair<int, int> targetSquare;
		vector<pair<int, int>> path = GetPath(*map, cursor->col, cursor->row, target->col, target->row);
		if(path.size() > cursor->selected->mov)
		{
			targetSquare = FindClosestAccessibleTile(*map, 
							   path[cursor->selected->mov].first, 
							   path[cursor->selected->mov].second);
		}
		else
		{
			targetSquare = FindClosestAccessibleTile(*map, target->col, target->row);
		}

        // move cursor
        cursor->col = targetSquare.first;
        cursor->row = targetSquare.second;

        MoveViewport(targetSquare.first, targetSquare.second);

        GlobalAIState = FOUND_NEW_POSITION;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};


class AIPlaceUnitCommand : public Command
{
public:
    AIPlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;

        // Determine interactible squares
        map->attackable.clear();
        map->healable.clear();
        map->attackable.clear();
        vector<point> interactible = InteractibleFrom(*map, cursor->col, cursor->row, 
                                             cursor->selected->minRange, cursor->selected->maxRange);

        // attack
        for(const point &p : interactible)
        {
            if(map->tiles[p.first][p.second].occupant &&
               !map->tiles[p.first][p.second].occupant->isAlly)
            {
                map->attackable.push_back(p);
            }
        }

        // unit has to know its position as well
        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->sourceCol = cursor->col;
        cursor->sourceRow = cursor->row;

        cursor->selected->sheet.ChangeTrack(1);
    }

private:
    Cursor *cursor; 
    Tilemap *map;
};


// TODO: Make sure this code doesn't break if the enemy attacks a unit outside
// its current viewport.
class AIAttackTargetCommand : public Command
{
public:
    AIAttackTargetCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = FindVictim(*cursor, map);
        if(cursor->targeted)
        {
            int distance = ManhattanDistance(point(cursor->selected->col, cursor->selected->row),
                                             point(cursor->targeted->col, cursor->targeted->row));
            SimulateCombat(cursor->selected, cursor->targeted, distance,
                           map.tiles[cursor->selectedCol][cursor->selectedRow].avoid,
                           map.tiles[cursor->col][cursor->row].avoid);
        }

        cursor->selected->isExhausted = true;
        cursor->selected->sheet.ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        GlobalAIState = FINDING_NEXT;
    }

private:
    Cursor *cursor; 
    const Tilemap &map;
};


// ============================== struct ====================================
struct AI
{
    bool shouldPlan = true;

    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Tilemap *map)
    {
        shouldPlan = false;

        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, *map));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIMoveToClosestSquareCommand>(cursor, map));
        commandQueue.push(make_shared<AIPlaceUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIAttackTargetCommand>(cursor, *map));
    }

    void Update()
    {
        if(!commandQueue.empty())
        {
            commandQueue.front()->Execute();
            commandQueue.pop();
        }
        else
        {
            shouldPlan = true;
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
