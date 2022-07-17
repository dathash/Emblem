// Author: Alex Hartford
// Program: Emblem
// File: Commands
// Date: July 2022

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

    ATTACK_TARGETING_OVER_UNTARGETABLE,
    ATTACK_TARGETING_OVER_TARGET,

    HEALING_TARGETING_OVER_UNTARGETABLE,
    HEALING_TARGETING_OVER_TARGET,

    PREVIEW_ATTACK,
    PREVIEW_HEALING,

    GAME_MENU_ROOT,
    GAME_MENU_OUTLOOK,
    GAME_MENU_OPTIONS,

    UNIT_MENU_ROOT,
    UNIT_INFO,

    ENEMY_INFO,

    NO_OP,
};

static InterfaceState GlobalInterfaceState = NEUTRAL_OVER_GROUND;

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
    MoveCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in,
                TileInfo *tileInfo_in)
    : cursor(cursor_in),
      col(col_in),
      row(row_in),
      map(map_in),
      tileInfo(tileInfo_in)
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
            cursor->MoveViewport(newCol, newRow);

            if(map.tiles[newCol][newRow].occupied)
            {
                tileInfo->UpdateTextTextures({
                            map.tiles[newCol][newRow].occupant->name,
                            "Tile AVO: " + to_string(map.tiles[newCol][newRow].avoid)});
                tileInfo->hp = map.tiles[newCol][newRow].occupant->hp;
                tileInfo->maxHp = map.tiles[newCol][newRow].occupant->maxHp;
            }

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
    TileInfo *tileInfo;
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

        if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // For rendering
            cursor->MoveViewport(newCol, newRow);

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];
            if(VectorHasElement(pair<int, int>(newCol, newRow), map.accessible))
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
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = false;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;
        map->tiles[cursor->col][cursor->row].occupied = true;

        // unit has to know its position as well
        cursor->selected->col = cursor->col;
        cursor->selected->row = cursor->row;

        cursor->selected->sheet.ChangeTrack(1);

        // change state
        GlobalInterfaceState = UNIT_MENU_ROOT;

        // Move onto next level!
        if(map->tiles[cursor->col][cursor->row].type == OBJECTIVE)
        {
            printf("Objective Reached. Onto the next level!\n");
            GlobalNextLevel = true;
            GlobalTurnStart = true;
        }
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
        map->tiles[cursor->col][cursor->row].occupant = nullptr;
        map->tiles[cursor->col][cursor->row].occupied = false;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = cursor->selected;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = true;

        cursor->col = cursor->selectedCol;
        cursor->row = cursor->selectedRow;

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
class MoveAttackTargetingCommand : public Command
{
public:
    MoveAttackTargetingCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
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
            cursor->MoveViewport(newCol, newRow);

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];
            if(VectorHasElement(pair<int, int>(newCol, newRow), map.interactible) &&
               hoverTile->occupied &&
               !hoverTile->occupant->isAlly)
            {
                GlobalInterfaceState = ATTACK_TARGETING_OVER_TARGET;
            }

            else
            {
                GlobalInterfaceState = ATTACK_TARGETING_OVER_UNTARGETABLE;
            }
        }
    }

private:
    Cursor *cursor; 
    int col;
    int row;
    const Tilemap &map;
};

class MoveHealingTargetingCommand : public Command
{
public:
    MoveHealingTargetingCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
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
            cursor->MoveViewport(newCol, newRow);

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];
            if(VectorHasElement(pair<int, int>(newCol, newRow), map.interactible) &&
               hoverTile->occupied &&
               hoverTile->occupant->isAlly)
            {
                GlobalInterfaceState = HEALING_TARGETING_OVER_TARGET;
            }
            else
            {
                GlobalInterfaceState = HEALING_TARGETING_OVER_UNTARGETABLE;
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
    InitiateAttackCommand(Cursor *cursor_in, const Tilemap &map_in, CombatInfo *combatInfo_in)
    : cursor(cursor_in),
      map(map_in),
      combatInfo(combatInfo_in)
    {}


    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        combatInfo->UpdatePreview(*cursor->selected, *cursor->targeted);

        // change state
        GlobalInterfaceState = PREVIEW_ATTACK;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    CombatInfo *combatInfo;
};

class InitiateHealingCommand : public Command
{
public:
    InitiateHealingCommand(Cursor *cursor_in, const Tilemap &map_in, CombatInfo *combatInfo_in)
    : cursor(cursor_in),
      map(map_in),
      combatInfo(combatInfo_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        combatInfo->UpdatePreview(*cursor->selected, *cursor->targeted);

        // change state
        GlobalInterfaceState = PREVIEW_HEALING;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    CombatInfo *combatInfo;
};

class AttackCommand : public Command
{
public:
    AttackCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        SimulateCombat(cursor->selected, cursor->targeted);

        cursor->selected->isExhausted = true;
        cursor->selected->sheet.ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->col = cursor->sourceCol;
        cursor->row = cursor->sourceRow;

        // change state
        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }

private:
    Cursor *cursor;
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

        // change state
        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
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
        GlobalInterfaceState = ATTACK_TARGETING_OVER_TARGET;
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
        GlobalInterfaceState = HEALING_TARGETING_OVER_TARGET;
    }

private:
    Cursor *cursor;
};


// ======================= selecting enemy commands ==========================
class SelectEnemyCommand : public Command
{
public:
    SelectEnemyCommand(Cursor *cursor_in, Tilemap *map_in, UnitInfo *unitInfo_in)
    : cursor(cursor_in),
      map(map_in),
      unitInfo(unitInfo_in)
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

        unitInfo->UpdateTextTextures({cursor->selected->name,
                                      "ID: " + to_string(cursor->selected->id),
                                      "col: " + to_string(cursor->selected->col),
                                      "row: " + to_string(cursor->selected->row),
                                      "Ally: " + to_string(cursor->selected->isAlly),
                                      "Exhausted: " + to_string(cursor->selected->isExhausted),
                                      "Health: " + to_string(cursor->selected->hp) + " / " + to_string(cursor->selected->maxHp),
                                      "Movement: " + to_string(cursor->selected->mov),
                                      "Attack: " + to_string(cursor->selected->attack),
                                      "Defense: " + to_string(cursor->selected->defense),
                                      "Healing: " + to_string(cursor->selected->healing),
                                      "Range: " + to_string(cursor->selected->minRange) + "-" + to_string(cursor->selected->maxRange),
                                      "Accuracy: " + to_string(cursor->selected->accuracy)
                                      });                                        
    }

private:
    Cursor *cursor;
    Tilemap *map;
    UnitInfo *unitInfo;
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
    // TODO: CHANGE STATE?

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
    ChooseUnitMenuOptionCommand(Cursor *cursor_in, Tilemap *map_in, const Menu &menu_in, UnitInfo *unitInfo_in)
    : cursor(cursor_in),
      map(map_in),
      menu(menu_in),
      unitInfo(unitInfo_in)
    {}

    virtual void Execute()
    { 
        switch(menu.current)
        {
            case(0): // INFO
            {
                unitInfo->UpdateTextTextures({cursor->selected->name,
                                              "ID: " + to_string(cursor->selected->id),
                                              "col: " + to_string(cursor->selected->col),
                                              "row: " + to_string(cursor->selected->row),
                                              "Ally: " + to_string(cursor->selected->isAlly),
                                              "Exhausted: " + to_string(cursor->selected->isExhausted),
                                              "Health: " + to_string(cursor->selected->hp) + "/" + to_string(cursor->selected->maxHp),
                                              "Movement: " + to_string(cursor->selected->mov),
                                              "Attack: " + to_string(cursor->selected->attack),
                                              "Defense: " + to_string(cursor->selected->defense),
                                              "Healing: " + to_string(cursor->selected->healing),
                                              "Range: " + to_string(cursor->selected->minRange) + "-" + to_string(cursor->selected->maxRange),
                                              "Accuracy: " + to_string(cursor->selected->accuracy)
                                              });

                GlobalInterfaceState = UNIT_INFO;
            } break;
            case(1): // ATTACK
            {
                cursor->sourceCol = cursor->col;
                cursor->sourceRow = cursor->row;
                map->interactible.clear();
                map->interactible = InteractibleFrom(*map, cursor->sourceCol, cursor->sourceRow, 
                                                 cursor->selected->minRange, cursor->selected->maxRange);
                GlobalInterfaceState = ATTACK_TARGETING_OVER_UNTARGETABLE;
            } break;
            case(2): // HEAL
            {
                cursor->sourceCol = cursor->col;
                cursor->sourceRow = cursor->row;
                // Find tiles that a unit can interact with.

                map->interactible.clear();
                map->interactible = InteractibleFrom(*map, cursor->sourceCol, cursor->sourceRow, 
                                                     1, 1);

                GlobalInterfaceState = HEALING_TARGETING_OVER_UNTARGETABLE;
            } break;
            case(3): // WAIT
            {
                cursor->selected->isExhausted = true;

                cursor->selected->sheet.ChangeTrack(0);
                cursor->selected = nullptr;

                cursor->selectedCol = -1;
                cursor->selectedRow = -1;
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
            } break;
            default:
            {
                assert(!"ChooseUnitMenuOptionCommand | How did you get here?\n");
            } break;
        }
    }

private:
    Cursor *cursor;
    Tilemap *map;
    const Menu &menu;
    UnitInfo *unitInfo;
};

class BackOutOfUnitInfoCommand : public Command
{
public:
    virtual void Execute()
    {
        // change state
        GlobalInterfaceState = UNIT_MENU_ROOT;
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

        return nullptr;
    }

    InputHandler(Cursor *cursor, const Tilemap &map, TileInfo *tileInfo)
    {
        BindUp(make_shared<MoveCommand>(cursor, 0, -1, map, tileInfo));
        BindDown(make_shared<MoveCommand>(cursor, 0, 1, map, tileInfo));
        BindLeft(make_shared<MoveCommand>(cursor, -1, 0, map, tileInfo));
        BindRight(make_shared<MoveCommand>(cursor, 1, 0, map, tileInfo));
        BindA(make_shared<OpenGameMenuCommand>());
        BindB(make_shared<NullCommand>());
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

    // updates what the user can do with their buttons.
    // contains some state: the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor, Tilemap *map, 
                        Menu *gameMenu, Menu *unitMenu,
                        UnitInfo *unitInfo, TileInfo *tileInfo,
                        CombatInfo *combatInfo)
    {
        switch(GlobalInterfaceState)
        {
            case(NEUTRAL_OVER_GROUND):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map, tileInfo));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map, tileInfo));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map, tileInfo));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map, tileInfo));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map, tileInfo));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map, tileInfo));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map, tileInfo));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map, tileInfo));
                BindA(make_shared<SelectEnemyCommand>(cursor, map, unitInfo));
                BindB(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, 0, -1, *map, tileInfo));
                BindDown(make_shared<MoveCommand>(cursor, 0, 1, *map, tileInfo));
                BindLeft(make_shared<MoveCommand>(cursor, -1, 0, *map, tileInfo));
                BindRight(make_shared<MoveCommand>(cursor, 1, 0, *map, tileInfo));
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
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;

            case(SELECTED_OVER_ENEMY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveSCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveSCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveSCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>()); // TODO: Move and Attack?
                BindB(make_shared<DeselectUnitCommand>(cursor));
            } break;


            case(ATTACK_TARGETING_OVER_UNTARGETABLE):
            {
                BindUp(make_shared<MoveAttackTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveAttackTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveAttackTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveAttackTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(ATTACK_TARGETING_OVER_TARGET):
            {
                BindUp(make_shared<MoveAttackTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveAttackTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveAttackTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveAttackTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<InitiateAttackCommand>(cursor, *map, combatInfo));
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;

            case(HEALING_TARGETING_OVER_UNTARGETABLE):
            {
                BindUp(make_shared<MoveHealingTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveHealingTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveHealingTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveHealingTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(HEALING_TARGETING_OVER_TARGET):
            {
                BindUp(make_shared<MoveHealingTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveHealingTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveHealingTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveHealingTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<InitiateHealingCommand>(cursor, *map, combatInfo));
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;

            case(PREVIEW_ATTACK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<AttackCommand>(cursor));
                BindB(make_shared<BackDownFromAttackingCommand>(cursor));
            } break;
            case(PREVIEW_HEALING):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<HealCommand>(cursor));
                BindB(make_shared<BackDownFromHealingCommand>(cursor));
            } break;

            case(GAME_MENU_ROOT):
            {
                BindUp(make_shared<UpdateGameMenuCommand>(gameMenu, -1));
                BindDown(make_shared<UpdateGameMenuCommand>(gameMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseGameMenuOptionCommand>(gameMenu));
                BindB(make_shared<ExitGameMenuCommand>());
            } break;
            case(GAME_MENU_OUTLOOK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<NullCommand>());
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


            case(UNIT_MENU_ROOT):
            {
                BindUp(make_shared<UpdateUnitMenuCommand>(unitMenu, -1));
                BindDown(make_shared<UpdateUnitMenuCommand>(unitMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseUnitMenuOptionCommand>(cursor, map, *unitMenu, unitInfo));
                BindB(make_shared<UndoPlaceUnitCommand>(cursor, map));
            } break;
            case(UNIT_INFO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackOutOfUnitInfoCommand>());
            } break;
            case(ENEMY_INFO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectEnemyCommand>(cursor));
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

    queue<shared_ptr<Command>> commandQueue = {};
};

#endif
