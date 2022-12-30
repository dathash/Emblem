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
            printf("AI COMMAND | Move Cursor from <%d, %d> to <%d, %d>\n", 
                cursor->col, cursor->row, selected->col, selected->row);

            // move cursor
            cursor->col = selected->col;
            cursor->row = selected->row;
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

        printf("AI COMMAND | Select Unit %d at <%d, %d>\n", 
               cursor->selected->id, cursor->col, cursor->row);

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
        printf("COMMAND | Deactivate Unit %d at <%d, %d>\n",
               cursor->selected->id, cursor->col, cursor->row);
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

        printf("AI COMMAND | Move Cursor from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, targetSquare.first, targetSquare.second);

        // move cursor
        cursor->col = targetSquare.first;
        cursor->row = targetSquare.second;

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
        printf("AI COMMAND | Place Unit %d at <%d, %d>\n",
               cursor->selected->id, cursor->col, cursor->row); 
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;

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


class AIAttackTargetCommand : public Command
{
public:
    AIAttackTargetCommand(Cursor *cursor_in, Tilemap *map_in, CombatResolver *combatResolver_in)
    : cursor(cursor_in),
      map(map_in),
      combatResolver(combatResolver_in)
    {}

    virtual void Execute()
    {
        map->interactible.clear();
        map->interactible = InteractibleFrom(*map, cursor->sourceCol, cursor->sourceRow,
                                         cursor->selected->minRange, cursor->selected->maxRange);

        cursor->targeted = FindVictim(*cursor, *map);
        if(cursor->targeted)
        {
            int dist = ManhattanDistance(point(cursor->selected->col, cursor->selected->row),
                                         point(cursor->targeted->col, cursor->targeted->row));
            bool counter = dist >= cursor->targeted->minRange && dist <= cursor->targeted->maxRange; 

            printf("AI COMMAND | Attack Unit %d!\n",
                   cursor->targeted->id);

            combatResolver->attacker = cursor->selected;
            combatResolver->victim = cursor->targeted;
            combatResolver->counterAttack = counter;

            GlobalResolvingAction = true;
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
    Tilemap *map;
    CombatResolver *combatResolver;
};


// ============================== struct ====================================
struct AI
{
    bool shouldPlan = true;

    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Tilemap *map, CombatResolver *combatResolver)
    {
        printf("Planning!\n");
        shouldPlan = false;

        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, *map));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIMoveToClosestSquareCommand>(cursor, map));
        commandQueue.push(make_shared<AIPlaceUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIAttackTargetCommand>(cursor, map, combatResolver));
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
