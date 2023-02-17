// Author: Alex Hartford
// Program: Emblem
// File: Commands

#ifndef COMMAND_H
#define COMMAND_H

// inherited class, contains virtual method for implementing in children.
class Command
{
public:
    virtual void Execute() = 0;
    virtual ~Command() = default;
};

class NullCommand : public Command
{
public:
    virtual void Execute() {}
};

// ============================ neutral mode commands ========================
class MoveCommand : public Command
{
public:
    MoveCommand(Cursor *cursor_in, const Tilemap &map_in, direction dir_in)
    : cursor(cursor_in),
      map(map_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = cursor->pos + dir;

        if(IsValid(new_pos))
        {
            cursor->MoveTo(new_pos, dir * -1);

            const Tile *hoverTile = &map.tiles[new_pos.col][new_pos.row];
            if(!hoverTile->occupant)
            {
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
                return;
            }

            if(hoverTile->occupant->is_exhausted)
            {
                GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                return;
            }

            if(hoverTile->occupant->IsAlly())
            {
                GlobalInterfaceState = NEUTRAL_OVER_UNIT;
                return;
            }

            GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
            return;
        }
    }

private: 
    Cursor *cursor;
    const Tilemap &map;
    direction dir;
};

class SelectUnitCommand : public Command
{
public:
    SelectUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        GlobalInterfaceState = SELECTED;
        EmitEvent(PICK_UP_UNIT_EVENT);

        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;
        cursor->selected->initial_pos = cursor->pos;

        map->accessible.clear();

        if(cursor->selected->has_moved)
            return;

        // TODO: This should be simpler
        int movement = 0; 
        map->accessible = Accessible(*map, cursor->pos,
                                     cursor->selected->movement,
                                     1,
                                     1, 
                                     cursor->selected->IsAlly());
    }

private:
    Cursor *cursor;
    Tilemap *map;
};



// ========================== selected mode commands =========================
class DeselectUnitCommand : public Command
{
public:
    DeselectUnitCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->PlaceAt(cursor->selected->pos);
        cursor->selected = nullptr;

        GlobalInterfaceState = NEUTRAL_OVER_UNIT;

        EmitEvent(PLACE_UNIT_EVENT);
    }

private:
    Cursor *cursor;
};

class MoveSelectedCommand : public Command
{
public:
    MoveSelectedCommand(Cursor *cursor_in, const Tilemap &map_in, direction dir_in)
    : cursor(cursor_in),
      map(map_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = cursor->pos + dir;

        if(!IsValid(new_pos))
            return;

        cursor->MoveTo(new_pos, dir * -1);

        if(!cursor->selected->has_moved && VectorHasElement(new_pos, map.accessible))
            cursor->path_draw = GetPath(map, cursor->selected->initial_pos, cursor->pos, true);
    }

private:
    Cursor *cursor; 
    const Tilemap &map;
    direction dir;
};

class PlaceUnitCommand : public Command
{
public:
    PlaceUnitCommand(Cursor *cursor_in, Level *level_in)
    : cursor(cursor_in),
      level(level_in)
    {}

    virtual void Execute()
    {
        if(cursor->selected->has_moved)
            return;

        level->map.accessible.clear();

        position pos = cursor->pos;
        if(!cursor->path_draw.empty())
        {
            pos = cursor->path_draw.back();
            cursor->path_draw = {};
        }

        level->map.tiles[cursor->selected->pos.col][cursor->selected->pos.row].occupant = nullptr;
        level->map.tiles[pos.col][pos.row].occupant = cursor->selected;

        cursor->selected->initial_pos = cursor->selected->pos;
        cursor->selected->pos = pos;

        cursor->selected->has_moved = true;

        cursor->selected->sheet.ChangeTrack(TRACK_ACTIVE);
        EmitEvent(PLACE_UNIT_EVENT);
    }

private:
    Cursor *cursor; 
    Level *level;
};


class UndoMovementsCommand : public Command
{
public:
    UndoMovementsCommand(Level *level_in, Cursor *cursor_in)
    : level(level_in),
      cursor(cursor_in)
    {}

    virtual void Execute()
    {
        // TODO: We want this to be one at a time, so that none of the units land on eachother's prior square.
        for(shared_ptr<Unit> unit : level->combatants)
        {
            if(unit->has_moved)
            {
                position goal = unit->initial_pos;
                assert(!level->map.tiles[goal.col][goal.row].occupant);

                level->map.tiles[unit->pos.col][unit->pos.row].occupant = nullptr;
                level->map.tiles[goal.col][goal.row].occupant = unit.get();

                unit->pos = goal;
                unit->initial_pos = {0, 0};
                unit->has_moved = false;
            }
        }

        if(cursor->selected)
        {
            cursor->selected->sheet.ChangeTrack(TRACK_IDLE);
            return;
        }


        Unit *over = level->map.tiles[cursor->pos.col][cursor->pos.row].occupant;
        if(!over)
        {
            GlobalInterfaceState = NEUTRAL_OVER_GROUND;
            return;
        }

        if(over->is_exhausted)
        {
            GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
            return;
        }

        if(over->IsAlly())
        {
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;
            return;
        }

        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
        return;
    }

private:
    Level *level;
    Cursor *cursor;
};


// ============================== Attacking ====================================
class StopAttackingCommand : public Command
{
public:
    StopAttackingCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    { 
        cursor->PlaceAt(cursor->selected->pos);
        GlobalInterfaceState = SELECTED;
    }

private:
    Cursor *cursor;
};

class MoveAttackingCommand : public Command
{
public:
    MoveAttackingCommand(Cursor *cursor_in, Tilemap *map_in, direction dir_in)
    : cursor(cursor_in),
      map(map_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = cursor->pos + dir;

        if(!IsValid(new_pos))
            return;

        cursor->MoveTo(new_pos, dir * -1);

        if(VectorHasElement(new_pos, map->range))
        {
            cursor->targeting = new_pos;
            map->attackable.clear();
            map->attackable = {new_pos};
        }

        if(cursor->targeting == position(-1, -1) || 
           new_pos == cursor->selected->pos)
        {
            map->attackable.clear();
            GlobalInterfaceState = ATTACK_THINKING;
            return;
        }

        GlobalInterfaceState = ATTACK_TARGETING;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    direction dir;
};

class SeekVictimsCommand : public Command
{
public:
    SeekVictimsCommand(Cursor *cursor_in, Level *level_in)
    : cursor(cursor_in),
      level(level_in)
    {}

    virtual void Execute()
    {
        cursor->pos = cursor->selected->pos;

        level->map.range.clear();

        vector<position> orthogonal = {};
        orthogonal = Orthogonal(level->map, cursor->pos);
        switch(cursor->selected->primary->type)
        {
            case EQUIP_NONE:
            {
            } break;
            case EQUIP_PUNCH:
            {
                for(const position &p : orthogonal)
                {
                    int distance = ManhattanDistance(cursor->selected->pos, p);
                    if(distance >= cursor->selected->primary->min_range && distance <= cursor->selected->primary->max_range
                       && Unobstructed(level->map, cursor->selected->pos, p))
                        level->map.range.push_back(p);
                }
            } break;
            case EQUIP_LINE_SHOT:
            {
                for(const position &p : orthogonal)
                {
                    if(Unobstructed(level->map, cursor->selected->pos, p))
                        level->map.range.push_back(p);
                }
            } break;
            case EQUIP_ARTILLERY:
            {
                for(const position &p : orthogonal)
                {
                    int distance = ManhattanDistance(cursor->selected->pos, p);
                    if(distance >= cursor->selected->primary->min_range && distance <= cursor->selected->primary->max_range)
                        level->map.range.push_back(p);
                }
            } break;
            case EQUIP_SELF_TARGET:
            {
            } break;
            case EQUIP_LEAP:
            {
                for(const position &p : orthogonal)
                {
                    int distance = ManhattanDistance(cursor->selected->pos, p);
                    if(distance >= cursor->selected->primary->min_range && distance <= cursor->selected->primary->max_range
                       && !level->map.tiles[p.col][p.row].occupant)
                        level->map.range.push_back(p);
                }
            } break;
            case EQUIP_LASER:
            {
            } break;
            default:
            {
            } break;
        }
        level->map.attackable.clear();

        GlobalInterfaceState = ATTACK_THINKING;
    }

private:
    Cursor *cursor;
    Level *level;
};

class AttackCommand : public Command
{
public:
    AttackCommand(Tilemap *map_in, Cursor *cursor_in)
    : map(map_in),
      cursor(cursor_in)
    {}

    virtual void Execute()
    {
        Simulate(map, *cursor->selected->primary, cursor->selected->pos, cursor->targeting);
        cursor->PlaceAt(cursor->selected->pos);
        cursor->selected->Deactivate();
        cursor->selected = nullptr;

        GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
    }

private:
    Tilemap *map;
    Cursor *cursor;
};

// ======================= selecting enemy commands ==========================
class SelectEnemyCommand : public Command
{
public:
    SelectEnemyCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;

        map->accessible.clear();
        map->accessible = Accessible(*map, cursor->pos,
                                     cursor->selected->movement,
                                     1,
                                     1, 
                                     cursor->selected->IsAlly());

        GlobalInterfaceState = ENEMY_RANGE;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

class DeselectEnemyCommand : public Command
{
public:
    DeselectEnemyCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->selected = nullptr;
        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
    }

private:
    Cursor *cursor;
};

// ======================= game menu commands =========================================
class OpenGameMenuCommand : public Command
{
public:

    virtual void Execute()
    { 
        GlobalInterfaceState = GAME_MENU;
    }
};

class ExitGameMenuCommand : public Command
{
public:

    virtual void Execute()
    { 
        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }
};

class ChooseGameMenuOptionCommand : public Command
{
public:
    ChooseGameMenuOptionCommand(Menu *menu_in, Level *level_in,
                                Cursor *cursor_in)
    : menu(menu_in),
      level(level_in),
      cursor(cursor_in)
    {}

    virtual void Execute()
    { 
        EmitEvent(SELECT_MENU_OPTION_EVENT);
        switch(menu->current)
        {
            case(0): // OPTIONS
            {
                GlobalInterfaceState = GAME_MENU_OPTIONS;
                return;
            } break;
            case(1): // END TURN
            {
                //cursor->selected = nullptr;
                GoToResolutionPhase();
                return;
            } break;
        }
    }

private:
    Menu *menu;
    Level *level;
    Cursor *cursor;
};

class BackToGameMenuCommand : public Command
{
public:

    virtual void Execute()
    { 
        GlobalInterfaceState = GAME_MENU;
    }
};

class UpdateMenuCommand : public Command
{
public:
    UpdateMenuCommand(Menu *menu_in, int direction_in)
    : menu(menu_in),
      direction(direction_in)
    {}

    virtual void Execute()
    { 
        EmitEvent(MOVE_MENU_EVENT);
        int newCurrent = menu->current + direction;
        if(newCurrent >= menu->rows)
            menu->current = 0;
        else if(newCurrent < 0)
            menu->current = menu->rows - 1;
        else
            menu->current = newCurrent;
    }

private:
    Menu *menu;
    int direction;
};

// ================================= Meta =====================================
class ToTitleScreenCommand : public Command
{
public:
    ToTitleScreenCommand()
    {}

    virtual void Execute()
    {
        GlobalInterfaceState = TITLE_SCREEN;
    }
};

class QuitGameCommand : public Command
{
public:
    QuitGameCommand()
    {}

    virtual void Execute()
    {
        GlobalRunning = false;
    }
};

class RestartGameCommand : public Command
{
public:
    RestartGameCommand(Level *level_in,
                       const vector<shared_ptr<Unit>> &units_in,
                       const vector<shared_ptr<Unit>> &party_in
                       )
    : level(level_in),
      units(units_in),
      party(party_in)
    {}

    virtual void Execute()
    {
        *level = LoadLevel(level->name, units, party);
        level->song->Restart();

        GoToAIPhase();
    }

private:
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
};

class StartGameCommand : public Command
{
public:
    StartGameCommand()
    {}

    virtual void Execute()
    {
        GoToAIPhase();
    }

private:
    Cursor *cursor;
};

// ============================== Input Handler ================================
class InputHandler
{
public:
    // abstraction layer. simply executes the given command.
    shared_ptr<Command> HandleInput(InputState *input)
    {
        if(!input->joystickCooldown)
        {
            if(input->up) {
                input->joystickCooldown = JOYSTICK_COOLDOWN_TIME;
                return buttonUp;
            }
            if(input->down) {
                input->joystickCooldown = JOYSTICK_COOLDOWN_TIME;
                return buttonDown;
            }
            if(input->left) {
                input->joystickCooldown = JOYSTICK_COOLDOWN_TIME;
                return buttonLeft;
            }
            if(input->right) {
                input->joystickCooldown = JOYSTICK_COOLDOWN_TIME;
                return buttonRight;
            }
        }
        if(input->a) {
            input->a = false;
            return buttonA;
        }
        if(input->b) {
            input->b = false;
            return buttonB;
        }
        if(input->l) {
            input->l = false;
            return buttonL;
        }
        if(input->r) {
            input->r = false;
            return buttonR;
        }

        return nullptr;
    }

    InputHandler(Cursor *cursor, const Tilemap &map)
    {
        BindUp(make_shared<MoveCommand>(cursor, map, direction(0, -1)));
        BindDown(make_shared<MoveCommand>(cursor, map, direction(0, 1)));
        BindLeft(make_shared<MoveCommand>(cursor, map, direction(-1, 0)));
        BindRight(make_shared<MoveCommand>(cursor, map, direction(1, 0)));
        BindA(make_shared<OpenGameMenuCommand>());
        BindB(make_shared<NullCommand>());
        BindL(make_shared<NullCommand>());
        BindR(make_shared<NullCommand>());
    }

    void Update(InputState *input)
    {
        shared_ptr<Command> newCommand = HandleInput(input);
        if(newCommand)
        {
            commandQueue.push(newCommand);
        }

        while(!commandQueue.empty())
        {
            commandQueue.front()->Execute();
            commandQueue.pop();
        }
    }

    void BindUp(shared_ptr<Command> command)
    {
        buttonUp = command;
    }
    void BindDown(shared_ptr<Command> command)
    {
        buttonDown = command;
    }
    void BindLeft(shared_ptr<Command> command)
    {
        buttonLeft = command;
    }
    void BindRight(shared_ptr<Command> command)
    {
        buttonRight = command;
    }
    void BindA(shared_ptr<Command> command)
    {
        buttonA = command;
    }
    void BindB(shared_ptr<Command> command)
    {
        buttonB = command;
    }
    void BindL(shared_ptr<Command> command)
    {
        buttonL = command;
    }
    void BindR(shared_ptr<Command> command)
    {
        buttonR = command;
    }

    // updates what the user can do with their buttons.
    // contains some state: the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor, Level *level, 
                        const vector<shared_ptr<Unit>> &units,
                        const vector<shared_ptr<Unit>> &party,
                        Menu *gameMenu
                       )
    {
        switch(GlobalInterfaceState)
        {
            case(NO_OP):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(TITLE_SCREEN):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<StartGameCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(GAME_OVER):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<RestartGameCommand>(level, units, party));
                BindB(make_shared<ToTitleScreenCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(GAME_MENU):
            {
                BindUp(make_shared<UpdateMenuCommand>(gameMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(gameMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseGameMenuOptionCommand>(gameMenu, level, cursor));
                BindB(make_shared<ExitGameMenuCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(GAME_MENU_OPTIONS):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackToGameMenuCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(ENEMY_RANGE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectEnemyCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_GROUND):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<SelectEnemyCommand>(cursor, &(level->map)));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<SelectUnitCommand>(cursor, &(level->map)));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_DEACTIVATED_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<NullCommand>());
            } break;
            case(SELECTED):
            {
                BindUp(make_shared<MoveSelectedCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveSelectedCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveSelectedCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveSelectedCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<PlaceUnitCommand>(cursor, level));
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<SeekVictimsCommand>(cursor, level));
            } break;

            case(ATTACK_THINKING):
            {
                BindUp(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, -1)));
                BindDown(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, 1)));
                BindLeft(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(-1, 0)));
                BindRight(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(1, 0)));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<StopAttackingCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(ATTACK_TARGETING):
            {
                BindUp(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, -1)));
                BindDown(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, 1)));
                BindLeft(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(-1, 0)));
                BindRight(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(1, 0)));
                BindA(make_shared<AttackCommand>(&(level->map), cursor));
                BindB(make_shared<StopAttackingCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(ATTACK_RESOLUTION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            default:
            {
                SDL_assert(!"Unimplemented GlobalInterfaceState!\n");
            } break;
        }
    }

    void clearQueue()
    {
        commandQueue = {};
    }

private:
    shared_ptr<Command> buttonUp;
    shared_ptr<Command> buttonDown;
    shared_ptr<Command> buttonLeft;
    shared_ptr<Command> buttonRight;
    shared_ptr<Command> buttonA;
    shared_ptr<Command> buttonB;
    shared_ptr<Command> buttonL;
    shared_ptr<Command> buttonR;

    queue<shared_ptr<Command>> commandQueue = {};
};

#endif
