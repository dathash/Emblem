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
    MoveCommand(Cursor *cursor_in, const Tilemap &map_in, direction dir_in)
    : cursor(cursor_in),
      map(map_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = cursor->pos + dir;

        if(IsValidBoundsPosition(map.width, map.height, new_pos))
        {
            // move cursor
            cursor->pos = new_pos;
            MoveViewport(new_pos);

            // change state
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

            if(hoverTile->occupant->is_ally)
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
        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;
        cursor->redo = cursor->pos;
        map->accessible = AccessibleFrom(*map, cursor->redo,
                                         cursor->selected->movement,
                                         cursor->selected->is_ally);

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
        cursor->pos = cursor->redo;
        cursor->selected = nullptr;

        cursor->path_draw = {};

        GlobalInterfaceState = NEUTRAL_OVER_UNIT;
    }

private:
    Cursor *cursor;
};


class MoveSCommand : public Command
{
public:
    MoveSCommand(Cursor *cursor_in, const Tilemap &map_in, direction dir_in)
    : cursor(cursor_in),
      map(map_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = {cursor->pos.col + dir.col, cursor->pos.row + dir.row};

        if(!IsValidBoundsPosition(map.width, map.height, new_pos))
            return;

        // move cursor
        cursor->pos = new_pos;
        MoveViewport(new_pos);

        const Tile *hoverTile = &map.tiles[new_pos.col][new_pos.row];
        if(!VectorHasElement(new_pos, map.accessible))
        {
            GlobalInterfaceState = SELECTED_OVER_INACCESSIBLE;
            return;
        }

        cursor->path_draw = GetPath(map, cursor->redo, cursor->pos, true);

        if(!hoverTile->occupant || hoverTile->occupant->ID() == cursor->selected->ID())
        {
            GlobalInterfaceState = SELECTED_OVER_GROUND;
            return;
        }
        if(hoverTile->occupant->is_ally)
        {
            GlobalInterfaceState = SELECTED_OVER_ALLY;
            return;
        }
        GlobalInterfaceState = SELECTED_OVER_ENEMY;
    }

private:
    Cursor *cursor; 
    const Tilemap &map;
    direction dir;
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
        map->tiles[cursor->redo.col][cursor->redo.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(1);

        // Determine interactible squares
        map->attackable.clear();
        map->healable.clear();
        map->range.clear();
        vector<position> interactible = InteractibleFrom(*map, cursor->pos,
                                             cursor->selected->min_range, cursor->selected->max_range);

        // Update unit menu with available actions
        *menu = Menu({});

        if(map->tiles[cursor->pos.col][cursor->pos.row].type == OBJECTIVE &&
           cursor->selected->ID() == LEADER_ID)
        {
            menu->AddOption("Capture");
        }

        // for attacking
        for(const position &p : interactible)
        {
            map->range.push_back(p);
            if(map->tiles[p.col][p.row].occupant &&
               !map->tiles[p.col][p.row].occupant->is_ally)
            {
                map->attackable.push_back(p);
            }
        }
        if(map->attackable.size() > 0)
            menu->AddOption("Attack");

        interactible = InteractibleFrom(*map, cursor->pos, 1, 1);
        // for healing
        for(const position &p : interactible)
        {
            if(map->tiles[p.col][p.row].occupant &&
               map->tiles[p.col][p.row].occupant->is_ally &&
               map->tiles[p.col][p.row].occupant->health < map->tiles[p.col][p.row].occupant->max_health)
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
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = nullptr;
        map->tiles[cursor->redo.col][cursor->redo.row].occupant = cursor->selected;

        cursor->pos = cursor->redo;

        cursor->path_draw = {};

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(0);

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
        if(map->attackable.size() == 1)
            return;

        if(forward)
            rotate(map->attackable.begin(), 
                   map->attackable.begin() + 1,
                   map->attackable.end());
        else
            rotate(map->attackable.begin(), 
                   map->attackable.begin() + map->attackable.size() - 1,
                   map->attackable.end());
        position next = map->attackable[0];

        // move cursor
        cursor->pos = next;

        // For rendering
        MoveViewport(next);
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
        if(map->healable.size() == 1)
            return;

        if(forward)
            rotate(map->healable.begin(), 
                   map->healable.begin() + 1,
                   map->healable.end());
        else
            rotate(map->healable.begin(), 
                   map->healable.begin() + map->healable.size() - 1,
                   map->healable.end());
        position next = map->healable[0];

        // move cursor
        cursor->pos = next;

        // For rendering
        MoveViewport(next);
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
        cursor->pos = cursor->source;

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
        cursor->targeted = map.tiles[cursor->pos.col][cursor->pos.row].occupant;

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
        cursor->targeted = map.tiles[cursor->pos.col][cursor->pos.row].occupant;

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
        int distance = ManhattanDistance(cursor->source, cursor->pos);
        SimulateCombat(cursor->selected, cursor->targeted, distance,
                       map.tiles[cursor->source.col][cursor->source.row].avoid,
                       map.tiles[cursor->pos.col][cursor->pos.row].avoid);

        cursor->selected->Deactivate();

        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->pos = cursor->source;

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

        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->pos = cursor->source;

        cursor->path_draw = {};

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
        cursor->source = {-1, -1};

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
        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;
        cursor->redo = cursor->pos;

        GlobalInterfaceState = ENEMY_INFO;
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
        cursor->pos = cursor->redo;
        cursor->selected = nullptr;
        cursor->redo = {-1, -1};

        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
    }

private:
    Cursor *cursor;
};


class EnemyRangeCommand : public Command
{
public:
    EnemyRangeCommand(Cursor *cursor_in, Tilemap *map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;
        cursor->redo = cursor->pos;

        map->accessible = AccessibleFrom(*map, cursor->redo,
                                         cursor->selected->movement,
                                         cursor->selected->is_ally);

        GlobalInterfaceState = ENEMY_RANGE;
    }

private:
    Cursor *cursor;
    Tilemap *map;
};

class EnemyUndoRangeCommand : public Command
{
public:
    EnemyUndoRangeCommand()
    {}

    virtual void Execute()
    {
        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
    }
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
    ChooseGameMenuOptionCommand(Menu *menu_in)
    : menu(menu_in)
    {}

    virtual void Execute()
    { 
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
                EndPlayerTurn();
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
        GlobalInterfaceState = GAME_MENU;
    }
};

// ============================= unit menu commands =======================
class UpdateMenuCommand : public Command
{
public:
    UpdateMenuCommand(Menu *menu_in, int direction_in)
    : menu(menu_in),
      direction(direction_in)
    {}

    virtual void Execute()
    { 
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

        if(option == "Capture")
        {
            cursor->selected->Deactivate();

            cursor->selected = nullptr;
            cursor->redo = {-1, -1};
            cursor->path_draw = {};

            // Move onto next level!
            GlobalInterfaceState = LEVEL_MENU;
            return;
        }

        if(option == "Attack")
        {
            assert(map.attackable.size());
            cursor->source = cursor->pos;

            cursor->pos = map.attackable[0];
            GlobalInterfaceState = ATTACK_TARGETING;
            return;
        }
        if(option == "Heal")
        {
            assert(map.healable.size());
            cursor->source = cursor->pos;

            cursor->pos = map.healable[0];
            GlobalInterfaceState = HEAL_TARGETING;
            return;
        }

        if(option == "Wait")
        {
            cursor->selected->Deactivate();

            cursor->selected = nullptr;
            cursor->redo = {-1, -1};
            cursor->path_draw = {};

            GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
            return;
        }

        assert(!"ERROR ChooseUnitMenuOptionCommand | How did you get here?\n");
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
        GlobalInterfaceState = UNIT_INFO;
    }
};

class BackOutOfUnitInfoCommand : public Command
{
public:
    virtual void Execute()
    {
        GlobalInterfaceState = NEUTRAL_OVER_UNIT;
    }
};

class ChooseLevelMenuOptionCommand : public Command
{
public:
    ChooseLevelMenuOptionCommand(const Menu &menu_in)
    : menu(menu_in)
    {}

    virtual void Execute()
    {
        string option = menu.optionText[menu.current];

        if(option == "Next")
        {
            NextLevel();
            return;
        }
        if(option == "Redo")
        {
            RestartLevel();
            return;
        }
        if(option == "Conv")
        {
            GlobalInterfaceState = CONVERSATION;
            return;
        }

        assert(!"ERROR ChooseLevelMenuOptionCommand | How did you get here?\n");
    }

private:
    const Menu &menu;
};

// ================================== Conversations ============================
class NextSentenceCommand : public Command
{
public:
    NextSentenceCommand(Conversation *conversation_in)
    : conversation(conversation_in)
    {}

    virtual void Execute()
    {
        conversation->Next();
        if(conversation->current >= conversation->prose.size())
        {
            conversation->First();
            //TODO: Conversations are a one-time thing, so remove them from the menu.
            //      While we're at it, make a vector of conversations per level
            //      and create a new menu in level_menu which displays possible
            //      conversations.
            GlobalInterfaceState = LEVEL_MENU;
        }
        conversation->ReloadTextures();
    }

private:
    Conversation *conversation;
};

// ================================= Game Over =================================
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
    RestartGameCommand()
    {}

    virtual void Execute()
    {
        RestartLevel();
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
        BindUp(make_shared<MoveCommand>(cursor, map, direction(0, -1)));
        BindDown(make_shared<MoveCommand>(cursor, map, direction(0, 1)));
        BindLeft(make_shared<MoveCommand>(cursor, map, direction(-1, 0)));
        BindRight(make_shared<MoveCommand>(cursor, map, direction(1, 0)));
        BindA(make_shared<OpenGameMenuCommand>());
        BindB(make_shared<NullCommand>());
        BindR(make_shared<NullCommand>());
    }

    void Update(InputState *input)
    {
        if(!GlobalPlayerTurn)
            return;

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
                        Menu *gameMenu, Menu *unitMenu,
                        Menu *levelMenu, Conversation *conversation)
    {
        if(!GlobalPlayerTurn)
            return;

        switch(GlobalInterfaceState)
        {
            case(NEUTRAL_OVER_GROUND):
            {
                BindUp(make_shared<MoveCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<EnemyRangeCommand>(cursor, map));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<SelectEnemyCommand>(cursor, map));
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<SelectUnitCommand>(cursor, map));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(NEUTRAL_OVER_DEACTIVATED_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(SELECTED_OVER_GROUND):
            {
                BindUp(make_shared<MoveSCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<PlaceUnitCommand>(cursor, map, unitMenu));
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_INACCESSIBLE):
            {
                BindUp(make_shared<MoveSCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ALLY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<NullCommand>()); // CONSIDER: Move and Heal?
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ENEMY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, *map, direction(1, 0)));
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

            case(GAME_MENU):
            {
                BindUp(make_shared<UpdateMenuCommand>(gameMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(gameMenu, 1));
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
                BindUp(make_shared<UpdateMenuCommand>(unitMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(unitMenu, 1));
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
                BindB(make_shared<NullCommand>());
                BindR(make_shared<DeselectEnemyCommand>(cursor));
            } break;
            case(ENEMY_RANGE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<EnemyUndoRangeCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(LEVEL_MENU):
            {
                BindUp(make_shared<UpdateMenuCommand>(levelMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(levelMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseLevelMenuOptionCommand>(*levelMenu));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(CONVERSATION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(conversation));
                BindB(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(GAME_OVER):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<RestartGameCommand>());
                BindB(make_shared<QuitGameCommand>());
                BindR(make_shared<NullCommand>());
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
