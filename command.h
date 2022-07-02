// Author: Alex Hartford
// Program: Emblem
// File: Commands
// Date: July 2022

/*
    TODO
    Specify the other cases of our FSM once things are further along.
 */

#ifndef COMMAND_H
#define COMMAND_H

enum InterfaceState
{
    NEUTRAL_OVER_GROUND,
    NEUTRAL_OVER_ENEMY,
    NEUTRAL_OVER_UNIT,

    SELECTED_OVER_GROUND,
    SELECTED_OVER_INACCESSIBLE,
    SELECTED_OVER_ALLY,
    SELECTED_OVER_ENEMY,

    TARGETING_OVER_GROUND,
    TARGETING_OVER_UNTARGETABLE,
    TARGETING_OVER_ALLY,
    TARGETING_OVER_ENEMY,

    GAME_MENU_ROOT,
    GAME_MENU_OPTIONS,
    GAME_MENU_SAVE,
    GAME_MENU_UNITS,

    UNIT_MENU_ROOT,
    UNIT_MENU_INFO,
    UNIT_MENU_ATTACK,
    UNIT_MENU_HEAL,
    UNIT_MENU_TRADE,
    UNIT_MENU_ITEM,

    ENEMY_MENU_ROOT,

    COMBAT,

    NO_OP,
};

static InterfaceState GlobalInterfaceState = NEUTRAL_OVER_GROUND;



// =============================== commands ====================================
// inherited class, contains virtual methods.
class Command
{
public:
    virtual ~Command() 
    {
        //printf("Command has been Destructed!\n");
    }

    virtual void Execute() = 0;
};

class NullCommand : public Command
{
public:
    virtual void Execute()
    {
        printf("COMMAND | Null\n");
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
        printf("COMMAND | Move Cursor from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];

            if(!hoverTile->occupied || hoverTile->occupant->isExhausted)
            {
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
            }
            else
            {
                if(hoverTile->occupant->isAlly)
                {
                    GlobalInterfaceState = NEUTRAL_OVER_UNIT;
                }
                else
                {
                    GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
                }
            }
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
        GlobalInterfaceState = SELECTED_OVER_GROUND;
        cursor->selected = map->tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;

        // Find tiles that a unit can access.
        map->accessible = AccessibleFrom(*map, cursor->selectedCol, cursor->selectedRow, cursor->selected->mov);

        printf("COMMAND | Select Unit %d at <%d, %d>\n", 
               cursor->selected->id, cursor->col, cursor->row);
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
        printf("COMMAND | Deselect Unit %d.\n",
            cursor->selected->id);

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;
        cursor->selected = nullptr;

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
        printf("COMMAND | Move Cursor (Checked) from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];
            char buffer[256];

            if(VectorHasElement(pair<int, int>(newCol, newRow), *map.accessible.get()))
            {
                if(!hoverTile->occupied || hoverTile->occupant->id == cursor->selected->id)
                {
                    GlobalInterfaceState = SELECTED_OVER_GROUND;
                }
                else
                {
                    if(hoverTile->occupant->isAlly)
                    {
                        GlobalInterfaceState = SELECTED_OVER_ALLY;
                    }
                    else
                    {
                        GlobalInterfaceState = SELECTED_OVER_ENEMY;
                    }
                }
            }
            else
            {
                GlobalInterfaceState = SELECTED_OVER_INACCESSIBLE;
            }
        }
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
    PlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        printf("COMMAND | Place Unit %d at <%d, %d>\n",
               cursor->selected->id, cursor->col, cursor->row);
        
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = false;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;
        map->tiles[cursor->col][cursor->row].occupied = true;

        //cursor->selectedCol = cursor->col;
        //cursor->selectedRow = cursor->row;

        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
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
        printf("COMMAND | Put Unit %d back at <%d, %d>\n",
               cursor->selected->id, cursor->selectedCol, cursor->selectedRow);
        
        map->tiles[cursor->col][cursor->row].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupied = false;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = cursor->selected;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = true;

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;

        GlobalInterfaceState = SELECTED_OVER_GROUND;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
};


class DeactivateUnitCommand : public Command
{
public:
    DeactivateUnitCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        printf("COMMAND | Deactivate Unit %d at <%d, %d>\n",
               cursor->selected->id, cursor->col, cursor->row);

        cursor->selected->isExhausted = true;
        cursor->selected = nullptr;

        cursor->selectedCol = -1;
        cursor->selectedRow = -1;

        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }

private:
    Cursor *cursor; 
};


// ============================== finding target ===========================================
class SelectTargetForAttackCommand : public Command
{
public:
    SelectTargetForAttackCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        // Find tiles that a unit can access.
        map->interactible = InteractibleFrom(*map, cursor->selectedCol, cursor->selectedRow, 
                                             cursor->selected->minRange, cursor->selected->maxRange);

        printf("COMMAND | Finding targets for Unit %d at <%d, %d>\n", 
               cursor->selected->id, cursor->col, cursor->row);
        GlobalInterfaceState = TARGETING_OVER_GROUND;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

class MoveTCommand : public Command
{
public:
    MoveTCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
    : cursor(cursor_in),
      col(col_in),
      row(row_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        printf("COMMAND | Move Cursor (Targeting) from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];

            if(VectorHasElement(pair<int, int>(newCol, newRow), *map.interactible.get()))
            {
                if(!hoverTile->occupied || hoverTile->occupant->id == cursor->selected->id)
                {
                    GlobalInterfaceState = TARGETING_OVER_GROUND;
                }
                else
                {
                    if(hoverTile->occupant->isAlly)
                    {
                        GlobalInterfaceState = TARGETING_OVER_ALLY;
                    }
                    else
                    {
                        GlobalInterfaceState = TARGETING_OVER_ENEMY;
                    }
                }
            }
            else
            {
                GlobalInterfaceState = TARGETING_OVER_UNTARGETABLE;
            }
        }
    }

private:
    Cursor *cursor; 
    int col;
    int row;
    const Tilemap &map;
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
        printf("COMMAND | Detarget\n");
        map->interactible = nullptr;

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;

        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

class AttackEnemyCommand : public Command
{
public:
    virtual void Execute() { printf("Attack Enemy!\n"); }
    // Change state to Attack Preview State.
};

class HealAllyCommand : public Command
{
public:
    virtual void Execute() { printf("Heal Ally!\n"); }
    // Change state to Healing Preview State.
};

class BackDownFromHealingCommand : public Command
{
public:
    virtual void Execute() { printf("Backed down from Healing Ally!\n"); }
    // Change state to Selected Over Enemy State.
};

class InitiateHealingCommand : public Command
{
public:
    virtual void Execute() { printf("Initiate Healing!\n"); }
    // Change state to Combat State.
};

class ChangeWeaponBeforeCombatCommand : public Command
{
public:
    virtual void Execute() { printf("Change Weapon!\n"); }
};

class BackDownFromCombatCommand : public Command
{
public:
    virtual void Execute() { printf("Don't Attack Enemy!\n"); }
    // Change state to Selected Over Enemy State.
};

class InitiateCombatCommand : public Command
{
public:
    virtual void Execute() { printf("Initiate Combat!\n"); }
    // Change state to Combat State.
};

class SelectedItemsCommand : public Command
{
public:
    virtual void Execute() { printf("Selecting Items!\n"); }
    // Change state to Unit Items State.
};

class BackOutOfItemsCommand : public Command
{
public:
    virtual void Execute() { printf("Back Out of Items!\n"); }
    // Change state to Unit Actions State.
};

class SelectedTradeCommand : public Command
{
public:
    virtual void Execute() { printf("Trading Items!\n"); }
    // Change state to Unit Trade State.
};

class BackOutOfTradeCommand : public Command
{
public:
    virtual void Execute() { printf("Back Out of Trade!\n"); }
    // Change state to Unit Actions State.
};

class SelectedUnitInfoCommand : public Command
{
public:
    virtual void Execute() { printf("Reading Unit Info!\n"); }
    // Change state to Unit Info State.
};

class BackOutOfUnitInfoCommand : public Command
{
public:
    virtual void Execute() { printf("Back Out of Unit Info!\n"); }
    // Change state to Unit Actions State.
};



// ======================= selecting enemy commands ==========================
class SelectEnemyCommand : public Command
{
public:
    SelectEnemyCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        GlobalInterfaceState = ENEMY_MENU_ROOT;
        cursor->selected = map.tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;

        printf("COMMAND | Select Enemy %d at <%d, %d>\n", 
            cursor->selected->id, cursor->col, cursor->row);
        printf(" <> INTERFACE | ally: %d, mov: %d, etc. <> \n",
               cursor->selected->isAlly, cursor->selected->mov);
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class DeselectEnemyCommand : public Command
{
public:
    DeselectEnemyCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        printf("COMMAND | Deselect Enemy %d.\n",
            cursor->selected->id);

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;
        cursor->selected = nullptr;

        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
    }

private:
    Cursor *cursor;
};

class NextPageCommand : public Command
{
public:
    virtual void Execute() { printf("Next Page!\n"); }
    // Change state a little bit? Or just go to next page.
};

// ======================= game menu commands =========================================

class OpenGameMenuCommand : public Command
{
public:
    virtual void Execute()
    { 
        printf("COMMAND | Open Game Menu\n");
        printf(" <> INTERFACE | options: 0, field: 0, save: 0, end turn: 0 <>\n");
        GlobalInterfaceState = GAME_MENU_ROOT;
    }
};

class ExitGameMenuCommand : public Command
{
public:
    virtual void Execute()
    { 
        printf("COMMAND | Exit Game Menu\n");
        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }
};

class UpdateGameMenuCommand : public Command
{
public:
    virtual void Execute()
    { 
        printf("COMMAND | Update Game Menu!\n");
        printf(" <> INTERFACE | options: 0, field: 0, save: 0, end turn: 0 <>\n");
    }
};
// TODO MORE GAME MENU COMMANDS GO HERE


// ============================ unit menu commands ==================================
class UpdateUnitMenuCommand : public Command
{
public:
    virtual void Execute()
    { 
        printf("COMMAND | Update Unit Menu!\n");
        printf(" <> INTERFACE | attack: 0, heal: 0, talk: 0, items: 0, \n <> trade: 0, wait: 0 <>\n");
    }
};
// TODO MORE UNIT MENU COMMANDS GO HERE



// ============================== Input Handler ================================

class InputHandler
{
public:
    // abstraction layer.
    // simply executes the given command.
    shared_ptr<Command> HandleInput(InputState *input)
    {
        if(input->up) {
            input->up = false;
            return buttonUp;
        }
        if(input->down) {
            input->down = false;
            return buttonDown;
        }
        if(input->left) {
            input->left = false;
            return buttonLeft;
        }
        if(input->right) {
            input->right = false;
            return buttonRight;
        }
        if(input->a) {
            input->a = false;
            return buttonA;
        }
        if(input->b) {
            input->b = false;
            return buttonB;
        }

        return NULL;
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

    // updates what the user can do with their buttons.
    // contains some state, which is the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor, Tilemap *map)
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
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<SelectEnemyCommand>(cursor, *map));
                BindB(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map));
                BindA(make_shared<SelectUnitCommand>(cursor, map));
                BindB(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_GROUND):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<PlaceUnitCommand>(cursor, map));
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;

            case(SELECTED_OVER_INACCESSIBLE):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;

            case(SELECTED_OVER_ALLY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<HealAllyCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;

            case(SELECTED_OVER_ENEMY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<AttackEnemyCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;


            case(TARGETING_OVER_GROUND):
            {
                BindUp(make_shared<MoveTCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(TARGETING_OVER_UNTARGETABLE):
            {
                BindUp(make_shared<MoveTCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(TARGETING_OVER_ALLY):
            {
                BindUp(make_shared<MoveTCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(TARGETING_OVER_ENEMY):
            {
                BindUp(make_shared<MoveTCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;


            case(GAME_MENU_ROOT):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<ExitGameMenuCommand>());
            } break;
            case(GAME_MENU_OPTIONS):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;
            case(GAME_MENU_SAVE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;
            case(GAME_MENU_UNITS):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;


            case(UNIT_MENU_ROOT):
            {
                BindUp(make_shared<DeactivateUnitCommand>(cursor));
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<SelectTargetForAttackCommand>(cursor, map));
                BindB(make_shared<UndoPlaceUnitCommand>(cursor, map));
            } break;
            case(UNIT_MENU_INFO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<BackOutOfUnitInfoCommand>());
                BindB(make_shared<BackOutOfUnitInfoCommand>());
            } break;
            case(UNIT_MENU_TRADE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;
            case(UNIT_MENU_ITEM):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;
            case(UNIT_MENU_ATTACK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;
            case(UNIT_MENU_HEAL):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;

            case(ENEMY_MENU_ROOT):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectEnemyCommand>(cursor));
            } break;

            case(COMBAT):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;


            case(NO_OP):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
            } break;


            default:
            {
                printf("Invalid Interface State! %d\n",
                       GlobalInterfaceState);

                assert(!"Unimplemented?\n");
            } break;
        }
    }

private:
    shared_ptr<Command> buttonUp;
    shared_ptr<Command> buttonDown;
    shared_ptr<Command> buttonLeft;
    shared_ptr<Command> buttonRight;
    shared_ptr<Command> buttonA;
    shared_ptr<Command> buttonB;
};



#endif
