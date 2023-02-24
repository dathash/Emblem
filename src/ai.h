// Author: Alex Hartford
// Program: Emblem
// File: AI

#ifndef AI_H
#define AI_H

// ============================= ai commands ================================
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
        map->accessible = Accessible(*map, cursor->pos,
                                     cursor->selected->movement,
                                     cursor->selected->IsAlly());

        GlobalAIState = AI_SELECTED;
    }
private:
    Cursor *cursor;
    Tilemap *map;
};

int EvaluateLocation(const Unit &unit, 
                     const position &pos, 
                     const Tilemap &map) {
    //if(map.tiles[pos.col][pos.row] is dangerous) return -10;
    if(IsEdge(pos)) return -5;

    if(unit.primary->type == EQUIP_PUNCH)
    {
        int distance = DistanceToNearest(map, pos,
                [](const Unit &unit) -> bool { return unit.IsAlly() || unit.IsEnv(); }, 
                false);
        return 6 - distance;
    }
    return 0;
}

int Evaluate(const Unit &unit, const position &pos, const direction &dir,
             const Tilemap &map) {
    switch(unit.primary->type)
    {
        case EQUIP_PUNCH:
        {
            int value = 0;
            for(int i = 1; i <= unit.primary->max_range; ++i)
            {
                position target = pos + (dir * i);

                if(!IsValid(target)) continue;

                Unit *victim = map.tiles[target.col][target.row].occupant;
                if(!victim || target == unit.pos) continue;

                switch(victim->team)
                {
                    case TEAM_PLAYER:   value += POINTS_FOR_PLAYER_UNIT; break;
                    case TEAM_ENV:      value += POINTS_FOR_BUILDING; break;
                    case TEAM_AI:       value += POINTS_FOR_AI_UNIT; break;
                }
            }
            return value;

        } break;
        case EQUIP_LINE_SHOT:
        {
            for(int i = 1; i <= 8; ++i)
            {
                position target = pos + (dir * i);

                if(!IsValid(target)) continue;

                Unit *victim = map.tiles[target.col][target.row].occupant;
                if(!victim || target == unit.pos) continue;

                switch(victim->team)
                {
                    case TEAM_PLAYER:   return POINTS_FOR_PLAYER_UNIT;
                    case TEAM_ENV:      return POINTS_FOR_BUILDING;
                    case TEAM_AI:       return POINTS_FOR_AI_UNIT;
                }
            }
            return 0;
        } break;
        case EQUIP_ARTILLERY:
        {
            return 0;
        } break;
        default: cout << "AI shouldn't have this kind of attack. " << unit.primary->type << "\n"; return 0;
    }
}

struct Action
{
    position move;
    position attack;
};

struct Choice
{
    Action action;
    int action_score;
    int location_score;
};

void
PrintChoice(const Choice &choice)
{
    cout << choice.action.move << " | " << choice.action.attack << " | " << choice.action_score << " | " << choice.location_score << "\n";
}

vector<Choice>
GetChoices(const Unit &unit, const Tilemap &map)
{
    vector<Choice> choices = {};
    vector<direction> directions = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}};

    for(const position &pos : map.accessible)
    {
        int value = Evaluate(unit, pos, directions[0], map);
        int best = 0;
        int best_value = value;

        for(int i = 1; i <= 3; ++i)
        {
            value = Evaluate(unit, pos, directions[i], map);
            if(value > best_value)
            {
                best_value = value;
                best = i;
            }
        }

        choices.push_back({{pos, pos + directions[best]},
                           best_value,
                           EvaluateLocation(unit, pos, map)});
    }
    return choices;
}

Choice
BestAction(const vector<Choice> &choices)
{
    int best_action_score = 0;
    int second_best_action_score = 0;
    vector<Choice> best_choices = {};
    vector<Choice> second_best_choices = {};
    for(const Choice &choice : choices)
    {
        if(choice.action_score > best_action_score)
        {
            second_best_action_score = best_action_score;
            second_best_choices = best_choices;

            best_action_score = choice.action_score;
            best_choices = {choice};
        }

        if(choice.action_score == best_action_score)
            best_choices.push_back(choice);

        if(choice.action_score == second_best_action_score)
            second_best_choices.push_back(choice);
    }

    return best_choices[RandomInt(best_choices.size()) - 1];
    // TODO: sub-optimal
}

Choice
BestLocation(const vector<Choice> &choices)
{
    // Location scoring (If nothing to attack.)
    int best_location_score = 0;
    vector<Choice> best_locations = {};
    for(const Choice &choice : choices)
    {
        if(choice.location_score > best_location_score)
        {
            best_location_score = choice.location_score;
            best_locations = {choice};
        }

        if(choice.location_score == best_location_score)
            best_locations.push_back(choice);
    }
    return best_locations[RandomInt(best_locations.size()) - 1];
}

// Scans the map and determines the best course of action to take.
Action GetAction(const Unit &unit, const Tilemap &map) {
    vector<Choice> choices = GetChoices(unit, map);
    assert(choices.size());

    Choice action_choice = BestAction(choices);
    if(action_choice.action_score != 0)
        return action_choice.action;

    return {BestLocation(choices).action.move, {-1, -1}}; // no attack
}

class AIPerformUnitActionCommand : public Command
{
public:
    AIPerformUnitActionCommand(Cursor *cursor_in, Tilemap *map_in,
                               Resolution *resolution_in)
    : cursor(cursor_in),
      map(map_in),
      resolution(resolution_in)
    {}

    virtual void Execute()
    {
        Action action = GetAction(*cursor->selected, *map);
        cursor->pos = action.move;

        map->tiles[cursor->selected->initial_pos.col][cursor->selected->initial_pos.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(TRACK_ACTIVE);

        // Prepare attack
        if(action.attack != position(-1, -1))
        {
            assert(cursor->selected->primary);
            resolution->attacks.push_back({cursor->selected, 
                                           action.attack - cursor->selected->pos});
            cursor->pos = action.attack;
        }
        else
        {
            cursor->pos = cursor->selected->pos;
        }

        cursor->selected->Deactivate();
        cursor->selected = nullptr;

        GlobalAIState = AI_FINDING_NEXT;
        return;
    }

private:
    Cursor *cursor;
    Tilemap *map;
    Resolution *resolution;
};

// ============================== struct ====================================
struct AI
{
    queue<shared_ptr<Command>> commandQueue = {};
    int frame = 0;


    // Fills the command queue with the current plan.
    void Plan(Cursor *cursor, Level *level, Resolution *resolution)
    {
        Unit *selected = nullptr;
        selected = FindNearest(level->map, cursor->pos,
                [](const Unit &unit) -> bool
                {
                    return unit.IsAI() && !unit.is_exhausted;
                }, false);
        if(selected)
        {
            cursor->pos = selected->pos;
            commandQueue.push(make_shared<AISelectUnitCommand>(cursor, &(level->map)));
            commandQueue.push(make_shared<AIPerformUnitActionCommand>(cursor, &(level->map), resolution));
        }
        else
        {
            GoToPlayerPhase(level, cursor);
        }
    }

    // Passes the args through to plan.
    void
    Update(Cursor *cursor, Level *level, Resolution *resolution)
    {
        if(GlobalAIState == AI_NO_OP ||
           GlobalAIState == AI_ATTACK_RESOLUTION)
            return;

        if(commandQueue.empty())
            Plan(cursor, level, resolution);

        ++frame;
        // Every __ frames.
        if(frame % AI_ACTION_SPEED)
            return;

        commandQueue.front()->Execute();
        commandQueue.pop();
    }
};

#endif
