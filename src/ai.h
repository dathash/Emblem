// Author: Alex Hartford
// Program: Emblem
// File: AI
// Date: July 2022

#ifndef AI_H
#define AI_H

enum AIState
{
    FINDING_NEXT,
    SELECTED,
    PLACED,

    ENEMY_TURN
};

static AIState GlobalAIState = ENEMY_TURN;

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
        shared_ptr<Unit> selected = nullptr;
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
            // TODO: Make this robust.
			printf("Out of units! Transitioning...\n");
            cursor->col = 0;
            cursor->row = 0;
            GlobalAIState = ENEMY_TURN;
            GlobalPlayerTurn = true;
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;
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

        cursor->selected->sheet->ChangeTrack(0);
        cursor->selected = nullptr;

        cursor->selectedCol = -1;
        cursor->selectedRow = -1;

        GlobalAIState = FINDING_NEXT;
    }

private:
    Cursor *cursor; 
};


//class AIMoveToClosestSquareCommand : public Command
//{
//public:
    //AIMoveToClosestSquareCommand(Cursor *cursor_in, Tilemap *map_in)
    //: cursor(cursor_in),
      //map(map_in)
    //{}

    //virtual void Execute()
    //{
    //}

//private:
    //Cursor *cursor;
    //Tilemap *map;
//};


//class AIPlaceUnitCommand : public Command
//{
//public:
    //AIPlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    //: cursor(cursor_in),
      //map(map_in)
    //{}

    //virtual void Execute()
    //{
        //printf("AI COMMAND | Place Unit %d at <%d, %d>\n",
               //cursor->selected->id, cursor->col, cursor->row); 
        //map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        //map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = false;
        //map->tiles[cursor->col][cursor->row].occupant = cursor->selected;
        //map->tiles[cursor->col][cursor->row].occupied = true;

        //// unit has to know its position as well
        //cursor->selected->col = cursor->col;
        //cursor->selected->row = cursor->row;

        //cursor->selected->sheet->ChangeTrack(1);
    //}

//private:
    //Cursor *cursor; 
    //Tilemap *map;
//};



// ============================== struct ====================================
struct AI
{
    queue<shared_ptr<Command>> commandQueue;
    bool shouldPlan = true;

    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Tilemap *map)
    {
        printf("Planning!\n");
        shouldPlan = false;

        commandQueue.push(make_shared<AIFindNextUnitCommand>(cursor, *map));
        commandQueue.push(make_shared<AISelectUnitCommand>(cursor, map));
        commandQueue.push(make_shared<AIDeactivateUnitCommand>(cursor));

        //shared_ptr<Unit> target = FindNearest(*cursor, *map, 
                //[](const Unit &unit) -> bool
                //{
                    //return unit.isAlly;
                //});
        //pair<int, int> targetSquare = FindClosestAccessibleTile(*map, target->col, target->row);
        //printf("%d %d\n", targetSquare.first, targetSquare.second);
        //commandQueue.push(make_shared<AIMoveCommand>(cursor, targetSquare.first, targetSquare.second));
        //commandQueue.push(make_shared<AIPlaceUnitCommand>(cursor, map));

        //}
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
};

#endif
