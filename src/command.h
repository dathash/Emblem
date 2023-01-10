// Author: Alex Hartford
// Program: Emblem
// File: Commands

#ifndef COMMAND_H
#define COMMAND_H

// =============================== commands ====================================
// inherited class, contains virtual method for implementing in children.
class Command
{
public:
    virtual void Execute() = 0;
};

class NullCommand : public Command
{
public:
    virtual void Execute()
    {
        printf("Null Command\n");
    }
};

// ============================ neutral mode commands ========================
class MoveCommand : public Command
{
public:
    MoveCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
    : cursor(cursor_in),
      col(col_in),
      row(row_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;

        if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // For rendering
            MoveViewport(newCol, newRow);

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];
            if(!hoverTile->occupant)
            {
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
                return;
            }

            if(hoverTile->occupant->isExhausted)
            {
                GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                return;
            }

            if(hoverTile->occupant->isAlly)
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
    int col;
    int row;
    const Tilemap &map;
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
        cursor->selected = map->tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;
        map->accessible.clear();
        map->accessible = AccessibleFrom(*map, cursor->selectedCol, 
                                         cursor->selectedRow, 
                                         cursor->selected->mov,
                                         cursor->selected->isAlly);

        // CONSIDER: Should I reset this when I finish using a unit, or
        // when I'm about to use one?
        //cursor->path_draw = {};

        // change state
        GlobalInterfaceState = SELECTED_OVER_GROUND;
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
        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;
        cursor->selected = nullptr;

        cursor->path_draw = {};

        // change state
        GlobalInterfaceState = NEUTRAL_OVER_UNIT;
    }

private:
    Cursor *cursor;
};


class MoveSCommand : public Command
{
public:
    MoveSCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
    : cursor(cursor_in),
      col(col_in),
      row(row_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;

        if(!IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            return;

        // move cursor
        cursor->col = newCol;
        cursor->row = newRow;
        MoveViewport(newCol, newRow);

        const Tile *hoverTile = &map.tiles[newCol][newRow];
        if(!VectorHasElement(point(newCol, newRow), map.accessible))
        {
            GlobalInterfaceState = SELECTED_OVER_INACCESSIBLE;
            return;
        }

        cursor->path_draw = GetPath(map, cursor->selectedCol, cursor->selectedRow,
                                    cursor->col, cursor->row);

        if(!hoverTile->occupant || hoverTile->occupant->id == cursor->selected->id)
        {
            GlobalInterfaceState = SELECTED_OVER_GROUND;
            return;
        }
        if(hoverTile->occupant->isAlly)
        {
            GlobalInterfaceState = SELECTED_OVER_ALLY;
            return;
        }
        GlobalInterfaceState = SELECTED_OVER_ENEMY;
    }

private:
    Cursor *cursor; 
    int col;
    int row;
    const Tilemap &map;
};

class PlaceUnitCommand : public Command
{
public:
    PlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in, Menu *menu_in)
    : cursor(cursor_in),
      map(map_in),
      menu(menu_in)
    {}

    virtual void Execute()
    {
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;

        if(cursor->selected->id == LEADER_ID)
        {
            leaderPosition = point(cursor->col, cursor->row);
        }

        // unit has to know its position as well (FOR NOW)
        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->selected->sheet.ChangeTrack(1);

        // Determine interactible squares
        map->attackable.clear();
        map->healable.clear();
        map->attackable.clear();
        vector<point> interactible = InteractibleFrom(*map, cursor->col, cursor->row, 
                                             cursor->selected->minRange, cursor->selected->maxRange);

        // Update unit menu with available actions
        *menu = Menu(0, 0, {});

        // attack
        for(const point &p : interactible)
        {
            if(map->tiles[p.first][p.second].occupant &&
               !map->tiles[p.first][p.second].occupant->isAlly)
            {
                map->attackable.push_back(p);
            }
        }
        if(map->attackable.size() > 0)
            menu->AddOption("Attack");

        interactible = InteractibleFrom(*map, cursor->col, cursor->row, 
                                        1, 1);
        // heal
        for(const point &p : interactible)
        {
            if(map->tiles[p.first][p.second].occupant &&
               map->tiles[p.first][p.second].occupant->isAlly &&
               map->tiles[p.first][p.second].occupant->hp < map->tiles[p.first][p.second].occupant->maxHp)
            {
                map->healable.push_back(p);
            }
        }
        if(map->healable.size() > 0)
            menu->AddOption("Heal");

        menu->AddOption("Wait");

        // change state
        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    Menu *menu;
};


class UndoPlaceUnitCommand : public Command
{
public:
    UndoPlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        map->tiles[cursor->col][cursor->row].occupant = nullptr;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = cursor->selected;

        if(cursor->selected->id == LEADER_ID)
        {
            leaderPosition = point(cursor->col, cursor->row);
        }

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;

        cursor->path_draw = {};

        // unit has to know its position as well
        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->selected->sheet.ChangeTrack(0);

        // change state
        GlobalInterfaceState = SELECTED_OVER_GROUND;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
};


// ============================== finding target ===========================================
class NextAttackTargetCommand : public Command
{
public:
    NextAttackTargetCommand(Cursor *cursor_in, Tilemap *map_in, bool forward_in)
    : cursor(cursor_in),
      map(map_in),
      forward(forward_in)
    {}

    virtual void Execute()
    {
        assert(map->attackable.size() > 0);
        if(map->attackable.size() > 1)
        {
            if(forward)
                rotate(map->attackable.begin(), 
                       map->attackable.begin() + 1,
                       map->attackable.end());
            else
                rotate(map->attackable.begin(), 
                       map->attackable.begin() + map->attackable.size() - 1,
                       map->attackable.end());
        }
        point next = map->attackable[0];

        // move cursor
        cursor->col = next.first;
        cursor->row = next.second;

        // For rendering
        MoveViewport(next.first, next.second);

        // change state
        GlobalInterfaceState = ATTACK_TARGETING;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
};

class NextHealTargetCommand : public Command
{
public:
    NextHealTargetCommand(Cursor *cursor_in, Tilemap *map_in, bool forward_in)
    : cursor(cursor_in),
      map(map_in),
      forward(forward_in)
    {}

    virtual void Execute()
    {
        assert(map->healable.size() > 0);
        if(map->healable.size() > 1)
        {
            if(forward)
                rotate(map->healable.begin(), 
                       map->healable.begin() + 1,
                       map->healable.end());
            else
                rotate(map->healable.begin(), 
                       map->healable.begin() + map->healable.size() - 1,
                       map->healable.end());
        }
        point next = map->healable[0];

        // move cursor
        cursor->col = next.first;
        cursor->row = next.second;

        // For rendering
        MoveViewport(next.first, next.second);

        // change state
        GlobalInterfaceState = HEAL_TARGETING;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
};

class DetargetCommand : public Command
{
public:
    DetargetCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    { 
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        // change state
        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

class InitiateAttackCommand : public Command
{
public:
    InitiateAttackCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}


    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        // change state
        GlobalInterfaceState = PREVIEW_ATTACK;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class InitiateHealCommand : public Command
{
public:
    InitiateHealCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        // change state
        GlobalInterfaceState = PREVIEW_HEALING;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class AttackCommand : public Command
{
public:
    AttackCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        int distance = ManhattanDistance(point(cursor->selected->col, cursor->selected->row),
                                         point(cursor->targeted->col, cursor->targeted->row));
        SimulateCombat(cursor->selected, cursor->targeted, distance,
                       map.tiles[cursor->selectedCol][cursor->selectedRow].avoid,
                       map.tiles[cursor->col][cursor->row].avoid);

        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        cursor->path_draw = {};

        GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};


class HealCommand : public Command
{
public:
    HealCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {

        SimulateHealing(cursor->selected, cursor->targeted);

        cursor->selected->isExhausted = true;
        cursor->selected->sheet.ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        cursor->path_draw = {};

        // change state
        GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
    }

private:
    Cursor *cursor;
};

class BackDownFromAttackingCommand : public Command
{
public:
    BackDownFromAttackingCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = nullptr;

        // change state
        GlobalInterfaceState = ATTACK_TARGETING;
    }

private:
    Cursor *cursor;
};

class BackDownFromHealingCommand : public Command
{
public:
    BackDownFromHealingCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = nullptr;

        // change state
        GlobalInterfaceState = HEAL_TARGETING;
    }

private:
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
        // change state
        GlobalInterfaceState = ENEMY_INFO;
        cursor->selected = map->tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;

        map->accessible.clear();
        map->accessible = AccessibleFrom(*map, cursor->selectedCol, 
                                         cursor->selectedRow, 
                                         cursor->selected->mov,
                                         cursor->selected->isAlly);
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
        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;
        cursor->selected = nullptr;

        // change state
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
        // change state
        GlobalInterfaceState = GAME_MENU_ROOT;
    }
};

class ExitGameMenuCommand : public Command
{
public:

    virtual void Execute()
    { 
        // change state
        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }
};


class UpdateGameMenuCommand : public Command
{
public:
    UpdateGameMenuCommand(Menu *menu_in, int direction_in)
    : menu(menu_in),
      direction(direction_in)
    {}

    virtual void Execute()
    { 
        int newCurrent = menu->current + direction;
        if(newCurrent >= menu->rows)
        {
            menu->current = 0;
        }
        else if(newCurrent < 0)
        {
            menu->current = menu->rows - 1;
        }
        else
        {
            menu->current = newCurrent;
        } 
    }

private:
    Menu *menu;
    int direction;
};


class ChooseGameMenuOptionCommand : public Command
{
public:
    ChooseGameMenuOptionCommand(Menu *menu_in)
    : menu(menu_in)
    {}

    virtual void Execute()
    { 
        // change state
        switch(menu->current)
        {
            case(0): // OUTLOOK
            {
                GlobalInterfaceState = GAME_MENU_OUTLOOK;
            } break;
            case(1): // OPTIONS
            {
                GlobalInterfaceState = GAME_MENU_OPTIONS;
            } break;
            case(2): // END TURN
            {
                GlobalInterfaceState = NO_OP;
                GlobalPlayerTurn = false;
                GlobalTurnStart = true;
            } break;
        }
    }

private:
    Menu *menu;
};

class BackToGameMenuCommand : public Command
{
public:

    virtual void Execute()
    { 
        // change state
        GlobalInterfaceState = GAME_MENU_ROOT;
    }
};

// ============================= unit menu commands =======================
class UpdateUnitMenuCommand : public Command
{
public:
    UpdateUnitMenuCommand(Menu *menu_in, int direction_in)
    : menu(menu_in),
      direction(direction_in)
    {}

    virtual void Execute()
    { 
        int newCurrent = menu->current + direction;
        if(newCurrent >= menu->rows)
        {
            menu->current = 0;
        }
        else if(newCurrent < 0)
        {
            menu->current = menu->rows - 1;
        }
        else
        {
            menu->current = newCurrent;
        } 
    }

private:
    Menu *menu;
    int direction;
};


class ChooseUnitMenuOptionCommand : public Command
{
public:
    ChooseUnitMenuOptionCommand(Cursor *cursor_in, const Tilemap &map_in, const Menu &menu_in)
    : cursor(cursor_in),
      map(map_in),
      menu(menu_in)
    {}

    virtual void Execute()
    {
        string option = menu.optionText[menu.current];

        if(option == "Attack")
        {
            assert(map.attackable.size());
            cursor->sourceCol = cursor->col;
            cursor->sourceRow = cursor->row;

            cursor->col = map.attackable[0].first;
            cursor->row = map.attackable[0].second;
            GlobalInterfaceState = ATTACK_TARGETING;
            return;
        }
        if(option == "Heal")
        {
            assert(map.healable.size());
            cursor->sourceCol = cursor->col;
            cursor->sourceRow = cursor->row;

            cursor->col = map.healable[0].first;
            cursor->row = map.healable[0].second;
            GlobalInterfaceState = HEAL_TARGETING;
            return;
        }

        if(option == "Wait")
        {
            cursor->selected->isExhausted = true;

            cursor->selected->sheet.ChangeTrack(0);
            cursor->selected = nullptr;

            cursor->selectedCol = -1;
            cursor->selectedRow = -1;

            cursor->path_draw = {};

            // Move onto next level!
            if(map.tiles[cursor->col][cursor->row].type == OBJECTIVE)
            {
                // TODO: Level Transitions
                printf("Objective Reached. Onto the next level!\n");
                GlobalNextLevel = true;
                GlobalTurnStart = true;
                return;
            }

            GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;

            return;
        }

        assert(!"ChooseUnitMenuOptionCommand | How did you get here?\n");
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    const Menu &menu;
};

class OpenUnitInfoCommand : public Command
{
public:
    virtual void Execute()
    {
        // change state
        GlobalInterfaceState = UNIT_INFO;
    }
};

class BackOutOfUnitInfoCommand : public Command
{
public:
    virtual void Execute()
    {
        // change state
        GlobalInterfaceState = NEUTRAL_OVER_UNIT;
    }
};

// ============================== Input Handler ================================
class InputHandler
{
public:
    // abstraction layer.
    // simply executes the given command.
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
        if(input->r) {
            input->r = false;
            return buttonR;
        }

        return nullptr;
    }

    InputHandler(Cursor *cursor, const Tilemap &map)
    {
        BindUp(make_shared<MoveCommand>(cursor, 0, -1, map));
        BindDown(make_shared<MoveCommand>(cursor, 0, 1, map));
        BindLeft(make_shared<MoveCommand>(cursor, -1, 0, map));
        BindRight(make_shared<MoveCommand>(cursor, 1, 0, map));
        BindA(make_shared<OpenGameMenuCommand>());
        BindB(make_shared<NullCommand>());
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

    // helper functions to bind Commands to each button.
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
    void BindR(shared_ptr<Command> command)
    {
        buttonR = command;
    }

    // updates what the user can do with their buttons.
    // contains some state: the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor, Tilemap *map,
                        Menu *gameMenu, Menu *unitMenu)
    {
        switch(GlobalInterfaceState)
        {
            case(NEUTRAL_OVER_GROUND):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<SelectEnemyCommand>(cursor, map));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<SelectEnemyCommand>(cursor, map));
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<SelectUnitCommand>(cursor, map));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(NEUTRAL_OVER_DEACTIVATED_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(SELECTED_OVER_GROUND):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<PlaceUnitCommand>(cursor, map, unitMenu));
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_INACCESSIBLE):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ALLY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>()); // CONSIDER: Move and Heal?
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ENEMY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>()); // CONSIDER: Move and Attack?
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(ATTACK_TARGETING):
            {
                BindUp(make_shared<NextAttackTargetCommand>(cursor, map, true));
                BindDown(make_shared<NextAttackTargetCommand>(cursor, map, false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateAttackCommand>(cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor, map));
                BindR(make_shared<NullCommand>());
            } break;
            case(HEAL_TARGETING):
            {
                BindUp(make_shared<NextHealTargetCommand>(cursor, map, true));
                BindDown(make_shared<NextHealTargetCommand>(cursor, map, false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateHealCommand>(cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor, map));
                BindR(make_shared<NullCommand>());
            } break;

            case(PREVIEW_ATTACK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<AttackCommand>(cursor, *map));
                BindB(make_shared<BackDownFromAttackingCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;
            case(PREVIEW_HEALING):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<HealCommand>(cursor));
                BindB(make_shared<BackDownFromHealingCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(GAME_MENU_ROOT):
            {
                BindUp(make_shared<UpdateGameMenuCommand>(gameMenu, -1));
                BindDown(make_shared<UpdateGameMenuCommand>(gameMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseGameMenuOptionCommand>(gameMenu));
                BindB(make_shared<ExitGameMenuCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(GAME_MENU_OUTLOOK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackToGameMenuCommand>());
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
                BindR(make_shared<NullCommand>());
            } break;

            case(UNIT_MENU_ROOT):
            {
                BindUp(make_shared<UpdateUnitMenuCommand>(unitMenu, -1));
                BindDown(make_shared<UpdateUnitMenuCommand>(unitMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseUnitMenuOptionCommand>(cursor, *map, *unitMenu));
                BindB(make_shared<UndoPlaceUnitCommand>(cursor, map));
                BindR(make_shared<NullCommand>());
            } break;
            case(UNIT_INFO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackOutOfUnitInfoCommand>());
                BindR(make_shared<BackOutOfUnitInfoCommand>());
            } break;
            case(ENEMY_INFO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectEnemyCommand>(cursor));
                BindR(make_shared<DeselectEnemyCommand>(cursor));
            } break;

            case(NO_OP):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            default:
            {
                assert(!"Unimplemented GlobalInterfaceState!\n");
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
    shared_ptr<Command> buttonR;

    queue<shared_ptr<Command>> commandQueue = {};
};

#endif
