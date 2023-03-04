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
    MoveCommand(Cursor *cursor_in, direction dir_in)
    : cursor(cursor_in),
      dir(dir_in)
    {}

    virtual void Execute()
    {
        position new_pos = cursor->pos + dir;

        if(!IsValid(new_pos))
            return;

        cursor->MoveTo(new_pos, dir * -1);
    }

private: 
    Cursor *cursor;
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
        if(!cursor->selected) return;

        if(cursor->selected->IsEnv()) return;
        else if(cursor->selected->IsAlly())
        {
            cursor->selected->initial_pos = cursor->pos;

            map->accessible.clear();
            if(cursor->selected->has_moved)
                return;

            bool paralyzed = cursor->selected->HasEffect(EFFECT_PARALYZED);
            int movement = (paralyzed ? 0 : cursor->selected->movement);
            movement = (cursor->selected->HasEffect(EFFECT_SWIFT) ? movement + 2 : movement);
            map->accessible = Accessible(*map, cursor->pos,
                                         movement,
                                         cursor->selected->IsAlly(),
                                         cursor->selected->HasEffect(EFFECT_SWIFT));

            GlobalInterfaceState = SELECTED;
            EmitEvent(PICK_UP_UNIT_EVENT);
        }
        else
        {
            cursor->selected = map->tiles[cursor->pos.col][cursor->pos.row].occupant;

            bool paralyzed = cursor->selected->HasEffect(EFFECT_PARALYZED);
            int movement = (paralyzed ? 0 : cursor->selected->movement);
            movement = (cursor->selected->HasEffect(EFFECT_SWIFT) ? movement + 2 : movement);
            map->accessible = Accessible(*map, cursor->pos,
                                         movement,
                                         cursor->selected->IsAlly(),
                                         cursor->selected->HasEffect(EFFECT_SWIFT));

            GlobalInterfaceState = ENEMY_RANGE;
        }
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

        GlobalInterfaceState = NEUTRAL;

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

        if(!cursor->selected->has_moved && VectorContains(new_pos, map.accessible))
            cursor->path_draw = GetPath(map, cursor->selected->initial_pos, cursor->pos, 
                                        true, 
                                        cursor->selected->HasEffect(EFFECT_SWIFT));
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
        // TODO: Undo doesn't work properly when your unit is placed on its source square.
        // Sol'n: perhaps have placing unit on their source square be a no-op?
        if(cursor->selected->has_moved)
            return;

        if(cursor->selected->initial_pos == cursor->pos)
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

        EffectType effect = level->map.tiles[pos.col][pos.row].effect;
        if(effect != EFFECT_NONE)
            cursor->selected->ApplyEffect(effect);

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
            if(unit->has_moved && !unit->is_exhausted)
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

        GlobalInterfaceState = NEUTRAL;
        return;
    }

private:
    Level *level;
    Cursor *cursor;
};

class UndoTurnCommand : public Command
{
public:
    UndoTurnCommand(Level *level_in, const Level &backup_in, 
                    Resolution *resolution_in, const Resolution &res_backup_in,
                    Cursor *cursor_in)
    : level(level_in),
      backup(backup_in),
      resolution(resolution_in),
      res_backup(res_backup_in),
      cursor(cursor_in)
    {}

    virtual void Execute()
    {
        if(!level->can_undo) return;

        *level = backup;
        cursor->pos = level->FirstAlly();
        cursor->Reset();

        cout << "BACKUP: " << res_backup.incidents.size() << "\n";
        *resolution = Resolution(res_backup);

        GlobalPlayer.health = GlobalPlayer.backup_health;
        level->can_undo = false;

        GlobalInterfaceState = NEUTRAL;
    }

private:
    Level *level;
    const Level &backup;
    Resolution *resolution;
    const Resolution &res_backup;
    Cursor *cursor;
};


// ============================== Attacking ====================================
class StopActingCommand : public Command
{
public:
    StopActingCommand(Cursor *cursor_in)
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

        if(VectorContains(new_pos, map->range))
        {
            cursor->targeting = new_pos;
            map->attackable = {new_pos};
        }

        if(cursor->with->type == EQUIP_SELF_TARGET)
        {
            GlobalInterfaceState = ATTACK_TARGETING;
            return;
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
    SeekVictimsCommand(Cursor *cursor_in, Level *level_in, Equip *with_in)
    : cursor(cursor_in),
      level(level_in),
      with(with_in)
    {}

    virtual void Execute()
    {
        if(!with) return;

        cursor->pos = cursor->selected->pos;
        cursor->targeting = {-1, -1};
        cursor->with = with;
        level->map.range.clear();
        level->map.attackable.clear();

        vector<position> orthogonal = {};
        orthogonal = Orthogonal(level->map, cursor->pos);
        switch(with->type)
        {
            case EQUIP_NONE:
            {
            } break;
            case EQUIP_PUNCH:
            {
                for(const position &p : orthogonal)
                {
                    int distance = ManhattanDistance(cursor->selected->pos, p);
                    if(distance >= with->min_range && distance <= with->max_range)
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
                    if(distance >= with->min_range && distance <= with->max_range)
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
                    if(distance >= with->min_range 
                       && distance <= with->max_range
                       && !level->map.tiles[p.col][p.row].occupant)
                    {
                        level->map.range.push_back(p);
                    }
                }
            } break;
            case EQUIP_LASER:
            {
            } break;
        }

        GlobalInterfaceState = ATTACK_THINKING;
    }

private:
    Cursor *cursor;
    Level *level;
    Equip *with;
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
        Simulate(map, *cursor->with, cursor->selected->pos, cursor->targeting);
        cursor->PlaceAt(cursor->selected->pos);
        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->with = nullptr;

        GlobalInterfaceState = NEUTRAL;
    }

private:
    Tilemap *map;
    Cursor *cursor;
};

// ======================= selecting enemy commands ==========================
class DeselectEnemyCommand : public Command
{
public:
    DeselectEnemyCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->selected = nullptr;
        GlobalInterfaceState = NEUTRAL;
    }

private:
    Cursor *cursor;
};

// ======================= game menu commands =========================================
class UndoCommand : public Command
{
public:

    virtual void Execute()
    { 
        GlobalInterfaceState = UNDO;
    }
};

class QueueCommand : public Command
{
public:

    virtual void Execute()
    {
        GlobalInterfaceState = QUEUE;
    }
};

class EndTurnCommand : public Command
{
public:
    EndTurnCommand(Level *level_in)
    : level(level_in)
    {}


    virtual void Execute()
    {
        level->TickEffects(TEAM_PLAYER);
        GoToResolutionPhase();
        return;
    }

private:
    Level *level;
};

class ReturnToNeutralCommand : public Command
{
public:
    virtual void Execute()
    { 
        GlobalInterfaceState = NEUTRAL;
    }
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

        GlobalPlayer.Reset();

        GlobalInterfaceState = WARP;
    }

private:
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
};

class NextLevelCommand : public Command
{
public:
    NextLevelCommand(Level *level_in,
                       const vector<shared_ptr<Unit>> &units_in,
                       const vector<shared_ptr<Unit>> &party_in,
                       const vector<string> &levels_in
                       )
    : level(level_in),
      units(units_in),
      party(party_in),
      levels(levels_in)
    {}

    virtual void Execute()
    {
        *level = LoadLevel(levels[RandomInt(levels.size() - 1)], units, party);
        level->song->Restart();

        GlobalPlayer.Reset();

        GlobalInterfaceState = WARP;
    }

private:
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
    const vector<string> &levels;
};

class StartGameCommand : public Command
{
public:
    StartGameCommand()
    {}

    virtual void Execute()
    {
        GlobalInterfaceState = WARP;
    }
};

// ================================= warp mode commands ========================
class WarpCommand : public Command
{
public:
    WarpCommand(Cursor *cursor_in, Level *level_in)
    : cursor(cursor_in),
      level(level_in)
    {}

    virtual void Execute()
    {
        const Unit *over = level->map.tiles[cursor->pos.col][cursor->pos.row].occupant;
        if(!Warpable(cursor->pos)) return;

        if(!over)
        {
            if(level->to_warp.empty()) return;

            level->AddCombatant(level->to_warp.back(), cursor->pos);
            level->to_warp.pop_back();
        }
        else
        {
            cout << "TODO! Can't swap warp spots yet.\n";
        }

        return;
    }

private: 
    Cursor *cursor;
    Level *level;
};


class ReadyCommand : public Command
{
public:
    ReadyCommand(Level *level_in)
    : level(level_in)
    {}


    virtual void Execute()
    {
        if(level->to_warp.empty())
            GoToAIPhase();
    }

private: 
    Level *level;
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
        if(input->x) {
            input->x = false;
            return buttonX;
        }
        if(input->y) {
            input->y = false;
            return buttonY;
        }

        return nullptr;
    }

    InputHandler(Cursor *cursor, const Tilemap &map)
    {
        BindUp(make_shared<NullCommand>());
        BindDown(make_shared<NullCommand>());
        BindLeft(make_shared<NullCommand>());
        BindRight(make_shared<NullCommand>());
        BindA(make_shared<NullCommand>());
        BindB(make_shared<NullCommand>());
        BindL(make_shared<NullCommand>());
        BindR(make_shared<NullCommand>());
        BindX(make_shared<NullCommand>());
        BindY(make_shared<NullCommand>());
    }

    void Update(InputState *input)
    {
        shared_ptr<Command> newCommand = HandleInput(input);
        if(newCommand) {
            commandQueue.push(newCommand);
        }

        while(!commandQueue.empty()) {
            commandQueue.front()->Execute();
            commandQueue.pop();
        }
    }

    void BindUp(shared_ptr<Command> command) {
        buttonUp = command;
    }
    void BindDown(shared_ptr<Command> command) {
        buttonDown = command;
    }
    void BindLeft(shared_ptr<Command> command) {
        buttonLeft = command;
    }
    void BindRight(shared_ptr<Command> command) {
        buttonRight = command;
    }
    void BindA(shared_ptr<Command> command) {
        buttonA = command;
    }
    void BindB(shared_ptr<Command> command) {
        buttonB = command;
    }
    void BindL(shared_ptr<Command> command) {
        buttonL = command;
    }
    void BindR(shared_ptr<Command> command) {
        buttonR = command;
    }
    void BindX(shared_ptr<Command> command) {
        buttonX = command;
    }
    void BindY(shared_ptr<Command> command) {
        buttonY = command;
    }

    // updates what the user can do with their buttons.
    // contains some state: the minimum amount.
    // each individual command takes only what is absolutely necessary for its completion.
    void UpdateCommands(Cursor *cursor,
                        Level *level, const Level &backup,
                        Resolution *resolution, const Resolution &res_backup,
                        const vector<shared_ptr<Unit>> &units,
                        const vector<shared_ptr<Unit>> &party,
                        const vector<string> &levels,
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
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;
            case(TITLE_SCREEN):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<StartGameCommand>());
                BindB(make_shared<NullCommand>());
                //BindB(make_shared<QuitGameCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
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
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;
            case(VICTORY):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextLevelCommand>(level, units, party, levels));
                BindB(make_shared<ToTitleScreenCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;

            case(QUEUE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<ReturnToNeutralCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;
            case(UNDO):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<UndoTurnCommand>(level, backup, resolution, res_backup, cursor));
                BindB(make_shared<ReturnToNeutralCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
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
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;

            case(NEUTRAL):
            {
                BindUp(make_shared<MoveCommand>(cursor, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, direction(1, 0)));
                BindA(make_shared<SelectUnitCommand>(cursor, &(level->map)));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<UndoMovementsCommand>(level, cursor));
                BindR(make_shared<UndoCommand>());
                BindX(make_shared<QueueCommand>());
                BindY(make_shared<EndTurnCommand>(level));
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
                BindR(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->utility));
                BindX(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->secondary));
                BindY(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->primary));
            } break;

            case(ATTACK_THINKING):
            {
                BindUp(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, -1)));
                BindDown(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, 1)));
                BindLeft(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(-1, 0)));
                BindRight(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(1, 0)));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<StopActingCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->utility));
                BindX(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->secondary));
                BindY(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->primary));
            } break;
            case(ATTACK_TARGETING):
            {
                BindUp(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, -1)));
                BindDown(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(0, 1)));
                BindLeft(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(-1, 0)));
                BindRight(make_shared<MoveAttackingCommand>(cursor, &(level->map), direction(1, 0)));
                BindA(make_shared<AttackCommand>(&(level->map), cursor));
                BindB(make_shared<StopActingCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->utility));
                BindX(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->secondary));
                BindY(make_shared<SeekVictimsCommand>(cursor, level, cursor->selected->primary));
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
                BindX(make_shared<NullCommand>());
                BindY(make_shared<NullCommand>());
            } break;

            case(WARP):
            {
                BindUp(make_shared<MoveCommand>(cursor, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, direction(1, 0)));
                BindA(make_shared<WarpCommand>(cursor, level));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
                BindX(make_shared<NullCommand>());
                BindY(make_shared<ReadyCommand>(level));
            } break;
        }
    }

    void clearQueue() {
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
    shared_ptr<Command> buttonX;
    shared_ptr<Command> buttonY;

    queue<shared_ptr<Command>> commandQueue = {};
};

#endif
