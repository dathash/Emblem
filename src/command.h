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

    TRADING_TARGETING_OVER_UNTARGETABLE,
    TRADING_TARGETING_OVER_TARGET,

    PREVIEW_ATTACK,
    PREVIEW_HEALING,
    TRADING,

    GAME_MENU_ROOT,
    GAME_MENU_OUTLOOK,
    GAME_MENU_OPTIONS,
    GAME_MENU_END_TURN,

    UNIT_MENU_ROOT,
    UNIT_INFO,
    UNIT_ITEMS,

    ENEMY_INFO,

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
    virtual shared_ptr<Tween> GetTween() = 0;
};

class NullCommand : public Command
{
public:
    virtual void Execute()
    {
        printf("COMMAND | Null\n");
    }

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        shared_ptr<Tween> result;
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        if(IsValidBoundsPosition(newCol, newRow))
        {
            if(col == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(col == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(row == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else if(row == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else
            {
                printf("MOVE COMMAND | You shouldn't be able to get here!\n");
            }
        }
        else
        {
            result = nullptr;
        }
        
        return result;
    }

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        cursor->selected = map->tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;
        map->accessible = AccessibleFrom(*map, cursor->selectedCol, 
                                         cursor->selectedRow, cursor->selected->mov);
        printf("COMMAND | Select Unit %d at <%d, %d>\n", 
               cursor->selected->id, cursor->col, cursor->row);

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        shared_ptr<Tween> result;
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        if(IsValidBoundsPosition(newCol, newRow))
        {
            if(col == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(col == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(row == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else if(row == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else
            {
                printf("MOVE COMMAND | You shouldn't be able to get here!\n");
            }
        }
        else
        {
            result = nullptr;
        }
        
        return result;
    }

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        printf("COMMAND | Place Unit %d at <%d, %d>\n",
               cursor->selected->id, cursor->col, cursor->row); 
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupant = nullptr;
        map->tiles[cursor->selectedCol][cursor->selectedRow].occupied = false;
        map->tiles[cursor->col][cursor->row].occupant = cursor->selected;
        map->tiles[cursor->col][cursor->row].occupied = true;

        cursor->selected->sheet->ChangeTrack(1);
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

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
        cursor->selected->sheet->ChangeTrack(0);
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        shared_ptr<Tween> result;
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        if(IsValidBoundsPosition(newCol, newRow))
        {
            if(col == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(col == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(row == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else if(row == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else
            {
                printf("MOVE COMMAND | You shouldn't be able to get here!\n");
            }
        }
        else
        {
            result = nullptr;
        }
        
        return result;
    }

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        printf("COMMAND | Move Cursor (Targeting for Attack) from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];

            if(VectorHasElement(pair<int, int>(newCol, newRow), *map.interactible) &&
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        shared_ptr<Tween> result;
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        if(IsValidBoundsPosition(newCol, newRow))
        {
            if(col == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(col == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(row == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else if(row == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else
            {
                printf("MOVE COMMAND | You shouldn't be able to get here!\n");
            }
        }
        else
        {
            result = nullptr;
        }
        
        return result;
    }

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        printf("COMMAND | Move Cursor (Targeting for healing) from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];

            if(VectorHasElement(pair<int, int>(newCol, newRow), *map.interactible) &&
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

class MoveTradingTargetingCommand : public Command
{
public:
    MoveTradingTargetingCommand(Cursor *cursor_in, int col_in, int row_in, const Tilemap &map_in)
    : cursor(cursor_in),
      col(col_in),
      row(row_in),
      map(map_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        shared_ptr<Tween> result;
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        if(IsValidBoundsPosition(newCol, newRow))
        {
            if(col == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(col == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->colPixelOffset, true);
            }
            else if(row == -1)
            {
                result = make_shared<Tween>(0, -TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else if(row == 1)
            {
                result = make_shared<Tween>(0, TILE_SIZE, CURSOR_MOVE_SPEED, &cursor->sheet->rowPixelOffset, true);
            }
            else
            {
                printf("MOVE COMMAND | You shouldn't be able to get here!\n");
            }
        }
        else
        {
            result = nullptr;
        }
        
        return result;
    }

    virtual void Execute()
    {
        int newCol = cursor->col + col;
        int newRow = cursor->row + row;
        printf("COMMAND | Move Cursor (Targeting for trading) from <%d, %d> to <%d, %d>\n", 
            cursor->col, cursor->row, newCol, newRow);

        if(IsValidBoundsPosition(newCol, newRow))
        {
            // move cursor
            cursor->col = newCol;
            cursor->row = newRow;

            // change state
            const Tile *hoverTile = &map.tiles[newCol][newRow];

            if(VectorHasElement(pair<int, int>(newCol, newRow), *map.interactible) &&
               hoverTile->occupied &&
               hoverTile->occupant->isAlly)
            {
                GlobalInterfaceState = TRADING_TARGETING_OVER_TARGET;
            }
            else
            {
                GlobalInterfaceState = TRADING_TARGETING_OVER_UNTARGETABLE;
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Detarget\n");
        map->interactible = nullptr;

        cursor->col = cursor->targeterCol;
        cursor->row = cursor->targeterRow;

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        printf("COMMAND | Initiate attack on Unit %d at <%d, %d>\n",
                cursor->targeted->id, cursor->col, cursor->row);

        GlobalInterfaceState = PREVIEW_ATTACK;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class InitiateHealingCommand : public Command
{
public:
    InitiateHealingCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        printf("COMMAND | Initiate healing on Unit %d at <%d, %d>\n",
                cursor->targeted->id, cursor->col, cursor->row);

        GlobalInterfaceState = PREVIEW_HEALING;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class InitiateTradingCommand : public Command
{
public:
    InitiateTradingCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->col][cursor->row].occupant;

        printf("COMMAND | Initiate trading on Unit %d at <%d, %d>\n",
                cursor->targeted->id, cursor->col, cursor->row);

        GlobalInterfaceState = TRADING;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};


class AttackCommand : public Command
{
public:
    AttackCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }
    
    virtual void Execute()
    {
        printf("COMMAND | Attack from Unit %d on Enemy %d. Simulating...\n", 
               cursor->selected->id, cursor->targeted->id);

        SimulateCombat(cursor->selected.get(), cursor->targeted.get());

        cursor->selected->isExhausted = true;
        cursor->selected->sheet->ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->col = cursor->targeterCol;
        cursor->row = cursor->targeterRow;

        GlobalInterfaceState = NEUTRAL_OVER_GROUND;
    }

private:
    Cursor *cursor;
};


// TODO: Make this system more generalizable
class TradeCommand : public Command
{
public:
    TradeCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }
    
    virtual void Execute()
    {
        // TRADING BEHAVIOR HERE

        printf("COMMAND | Traded between Unit %d and %d.\n", 
               cursor->selected->id, cursor->targeted->id);

        cursor->selected->isExhausted = true;
        cursor->selected->sheet->ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->col = cursor->targeterCol;
        cursor->row = cursor->targeterRow;

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        int damage = cursor->selected->healing;

        cursor->targeted->hp += damage;

        printf("COMMAND | Healing from Unit %d on Unit %d. %d healing.\n", 
               cursor->selected->id, cursor->targeted->id, damage);

        cursor->selected->isExhausted = true;
        cursor->selected->sheet->ChangeTrack(0);
        cursor->selected = nullptr;
        cursor->col = cursor->targeterCol;
        cursor->row = cursor->targeterRow;

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        printf("COMMAND | Backed down from attacking enemy.\n");

        cursor->targeted = nullptr;

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        printf("Command | Backed down from healing ally.\n");

        cursor->targeted = nullptr;

        GlobalInterfaceState = HEALING_TARGETING_OVER_TARGET;
    }

private:
    Cursor *cursor;
};

class BackDownFromTradingCommand : public Command
{
public:
    BackDownFromTradingCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        printf("COMMAND | Backed down from trading with ally.\n");

        cursor->targeted = nullptr;

        GlobalInterfaceState = TRADING_TARGETING_OVER_TARGET;
    }

private:
    Cursor *cursor;
};

class ChangeWeaponBeforeCombatCommand : public Command
{
public:
    virtual void Execute() { printf("Change Weapon!\n"); }

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }
};



// ======================= selecting enemy commands ==========================
class SelectEnemyCommand : public Command
{
public:
    SelectEnemyCommand(Cursor *cursor_in, const Tilemap &map_in, UnitInfo *unitInfo_in)
    : cursor(cursor_in),
      map(map_in),
      unitInfo(unitInfo_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    {
        GlobalInterfaceState = ENEMY_INFO;
        cursor->selected = map.tiles[cursor->col][cursor->row].occupant;
        cursor->selectedCol = cursor->col;
        cursor->selectedRow = cursor->row;

        unitInfo->UpdateTextTextures({"ID: " + to_string(cursor->selected->id),
                                      "Ally: " + to_string(cursor->selected->isAlly),
                                      "Exhausted: " + to_string(cursor->selected->isExhausted),
                                      "Health: " + to_string(cursor->selected->hp) + " / " + to_string(cursor->selected->maxHp),
                                      "Movement: " + to_string(cursor->selected->mov),
                                      "Attack: " + to_string(cursor->selected->attack),
                                      "Healing: " + to_string(cursor->selected->healing),
                                      "Range: " + to_string(cursor->selected->minRange) + " / " + to_string(cursor->selected->maxRange)
                                      });                                        

        printf("COMMAND | Select Enemy %d at <%d, %d>\n", 
            cursor->selected->id, cursor->col, cursor->row);
        printf(" <> INTERFACE | ally: %d, mov: %d, etc. <> \n",
               cursor->selected->isAlly, cursor->selected->mov);
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    UnitInfo *unitInfo;
};

class DeselectEnemyCommand : public Command
{
public:
    DeselectEnemyCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

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

// ======================= game menu commands =========================================

class OpenGameMenuCommand : public Command
{
public:

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Open Game Menu\n");
        GlobalInterfaceState = GAME_MENU_ROOT;
    }
};

class ExitGameMenuCommand : public Command
{
public:

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Exit Game Menu\n");
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Update Game Menu!\n");
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Choose Game Menu Option %d!\n", menu->current);        
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
                GlobalInterfaceState = GAME_MENU_END_TURN;
            } break;
        }
    }
    // TODO: CHANGE STATE?

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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Update Unit Menu!\n");
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

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }

    virtual void Execute()
    { 
        printf("COMMAND | Choose Unit Menu Option %d!\n", menu.current);
        switch(menu.current)
        {
            case(0): // INFO
            {
                unitInfo->UpdateTextTextures({"ID: " + to_string(cursor->selected->id),
                                              "Ally: " + to_string(cursor->selected->isAlly),
                                              "Exhausted: " + to_string(cursor->selected->isExhausted),
                                              "Health: " + to_string(cursor->selected->hp) + " / " + to_string(cursor->selected->maxHp),
                                              "Movement: " + to_string(cursor->selected->mov),
                                              "Attack: " + to_string(cursor->selected->attack),
                                              "Healing: " + to_string(cursor->selected->healing),
                                              "Range: " + to_string(cursor->selected->minRange) + " / " + to_string(cursor->selected->maxRange)
                                              });

                GlobalInterfaceState = UNIT_INFO;
            } break;
            case(1): // ITEMS
            {
                GlobalInterfaceState = UNIT_ITEMS;
            } break;
            case(2): // ATTACK
            {
                cursor->targeterCol = cursor->col;
                cursor->targeterRow = cursor->row;
                map->interactible = InteractibleFrom(*map, cursor->targeterCol, cursor->targeterRow, 
                                                 cursor->selected->minRange, cursor->selected->maxRange);
                printf(" > COMMAND | Finding targets to attack for Unit %d at <%d, %d>\n", 
                        cursor->selected->id, cursor->col, cursor->row);
                GlobalInterfaceState = ATTACK_TARGETING_OVER_UNTARGETABLE;
            } break;
            case(3): // HEAL
            {
                cursor->targeterCol = cursor->col;
                cursor->targeterRow = cursor->row;
                // Find tiles that a unit can interact with.

                map->interactible = InteractibleFrom(*map, cursor->targeterCol, cursor->targeterRow, 
                                                     1, 1);

                printf("COMMAND | Finding targets to heal for Unit %d at <%d, %d>\n", 
                        cursor->selected->id, cursor->col, cursor->row);
                GlobalInterfaceState = HEALING_TARGETING_OVER_UNTARGETABLE;
            } break;
            case(4): // TRADE
            {
                cursor->targeterCol = cursor->col;
                cursor->targeterRow = cursor->row;
                // Find tiles that a unit can interact with.

                map->interactible = InteractibleFrom(*map, cursor->targeterCol, cursor->targeterRow, 
                                                     1, 1);

                printf("COMMAND | Finding targets to trade with for Unit %d at <%d, %d>\n", 
                        cursor->selected->id, cursor->col, cursor->row);
                GlobalInterfaceState = TRADING_TARGETING_OVER_UNTARGETABLE;
            } break;
            case(5): // WAIT
            {
                printf("COMMAND | Deactivate Unit %d at <%d, %d>\n",
                       cursor->selected->id, cursor->col, cursor->row);
                cursor->selected->isExhausted = true;

                cursor->selected->sheet->ChangeTrack(0);
                cursor->selected = nullptr;

                cursor->selectedCol = -1;
                cursor->selectedRow = -1;
                GlobalInterfaceState = NEUTRAL_OVER_GROUND;
            } break;
            default:
            {
                printf("CHOOSEUNITMENUOPTIONCOMMAND | How did you get here?\n");
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
        printf("COMMAND | Back Out of Unit Info!\n");
        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
    }
};

class BackOutOfItemsMenuCommand : public Command
{
public:
    virtual void Execute()
    {
        printf("COMMAND | Back Out of Items Menu!\n");
        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

    virtual shared_ptr<Tween>
    GetTween()
    {
        return nullptr;
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
	// contains some state: the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor, Tilemap *map, 
                        Menu *gameMenu, Menu *unitMenu,
                        UnitInfo *unitInfo)
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
                BindA(make_shared<SelectEnemyCommand>(cursor, *map, unitInfo));
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
                BindA(make_shared<InitiateAttackCommand>(cursor, *map));
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
                BindA(make_shared<InitiateHealingCommand>(cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;

            case(TRADING_TARGETING_OVER_UNTARGETABLE):
            {
                BindUp(make_shared<MoveTradingTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTradingTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTradingTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTradingTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DetargetCommand>(cursor, map));
            } break;
            case(TRADING_TARGETING_OVER_TARGET):
            {
                BindUp(make_shared<MoveTradingTargetingCommand>(cursor, 0, -1, *map));
                BindDown(make_shared<MoveTradingTargetingCommand>(cursor, 0, 1, *map));
                BindLeft(make_shared<MoveTradingTargetingCommand>(cursor, -1, 0, *map));
                BindRight(make_shared<MoveTradingTargetingCommand>(cursor, 1, 0, *map));
                BindA(make_shared<InitiateTradingCommand>(cursor, *map));
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
            case(TRADING):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<TradeCommand>(cursor));
                BindB(make_shared<BackDownFromTradingCommand>(cursor));
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
            case(GAME_MENU_END_TURN):
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
            case(UNIT_ITEMS):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackOutOfItemsMenuCommand>());
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
