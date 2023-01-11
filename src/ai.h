// Author: Alex Hartford
// Program: Emblem
// File: AI

#ifndef AI_H
#define AI_H

void EndAITurn()
{
    GlobalAIState = PLAYER_TURN;
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


class AIPerformUnitActionCommand : public Command
{
public:
    AIPerformUnitActionCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
		// TODO: Choose Advantageous option based on simple rules.
			// 1. Minimum Enemy Health (pretty funny way to put it.)
			//	  Basically maximizes for kills and enemies who are already
			//	  weakened.  Note that this can be gamed, for instance with a
			//	  high DEF unit, get them to low health, and AI will ignore all
			//	  other units since minimum enemy health will always be on tank.
			// 2. High Likelihoods
			// 3. Minimum counterattack Damage
			// 4. Low enemy likelihoods
		// Right now, we just pick minimum enemy health.

		// =================================== Find ideal target =====================
        pair<point, Unit *> action = {};
		vector<pair<point, Unit *>> possibilities = FindAttackingSquares(*map, *cursor->selected);

		if(possibilities.size() == 0)
		{
			Unit *nearest = FindNearest(*cursor, *map,
				[](const Unit &unit) -> bool
				{
					return unit.isAlly;
				});
			path path_to_nearest = GetPath(*map, cursor->col, cursor->row, nearest->col, nearest->row, false);
			action = pair<point, Unit *>(path_to_nearest[cursor->selected->mov], NULL);
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
				outcome = PredictCombat(*cursor->selected, *t,
										ManhattanDistance(p, point(t->col, t->row)),
										map->tiles[p.first][p.second].avoid,
										map->tiles[t->col][t->row].avoid);
				if(outcome.two_health < min_health_after_attack)
				{
					action = poss;
					min_health_after_attack = outcome.two_health;
				}
			}
		}

		// ================================= Perform movement ========================
        // move cursor
        cursor->col = action.first.first;
        cursor->row = action.first.second;

        MoveViewport(cursor->col, cursor->row);

		// ================================= Place Unit ==============================
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;

        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->sourceCol = cursor->col;
        cursor->sourceRow = cursor->row;

        cursor->selected->sheet.ChangeTrack(1);

		// ================================= Perform Attack ==============================
        if(action.second)
        {
            int distance = ManhattanDistance(point(cursor->selected->col, cursor->selected->row),
                                             point(action.second->col, action.second->row));
            SimulateCombat(cursor->selected, action.second, distance,
                           map->tiles[cursor->selectedCol][cursor->selectedRow].avoid,
                           map->tiles[cursor->col][cursor->row].avoid);
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
        commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, map));
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
