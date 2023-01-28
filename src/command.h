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
    virtual ~Command() = default;
};

class NullCommand : public Command
{
public:
    virtual void Execute()
    {
        //printf("Null Command\n");
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
            cursor->MoveTo(new_pos, dir * -1);

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

class CycleUnitsCommand : public Command
{
public:
    CycleUnitsCommand(Cursor *cursor_in, Tilemap *map_in, bool forward_in)
    : cursor(cursor_in),
      map(map_in),
      forward(forward_in)
    {}

    virtual void Execute()
    {
        // TODO: Cycle units requires me to get the next allied unit.
        //       Solutions:
        //           Keep a vector of Allies<>, which works like attackable, etc.
        //           Split up combatants in Level into allies and enemies.
        //               (Obviously more of a change and harder, but might be the right thing)
        // move cursor
        cout << "Cycle Units UNIMPLEMENTED!\n";
        cursor->pos = cursor->pos;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
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
        // TODO: This condition should be dynamic based on what they do.
        // OR, we do what the new FE game does and overlay both with a sin wave between them.
        //if(cursor->selected->aptitude > cursor->selected->attack)
        //{
        //}
        //map->prospective = AccessibleFrom(*map, cursor->redo,
        //                                  cursor->selected->movement + cursor->selected->max_range,
        //                                  cursor->selected->is_ally);
        //cout << map->prospective.size() << "\n";

        EmitEvent(PICK_UP_UNIT_EVENT);

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
        cursor->PlaceAt(cursor->redo);
        cursor->selected = nullptr;

        cursor->path_draw = {};

        EmitEvent(PLACE_UNIT_EVENT);

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
        cursor->MoveTo(new_pos, dir * -1);

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
    PlaceUnitCommand(Cursor *cursor_in, Tilemap *map_in, Menu *menu_in,
                     ConversationList *conversations_in)
    : cursor(cursor_in),
      map(map_in),
      menu(menu_in),
      conversations(conversations_in)
    {}

    virtual void Execute()
    {
        map->tiles[cursor->redo.col][cursor->redo.row].occupant = nullptr;
        map->tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(1);

        // Determine interactible squares
        map->attackable.clear();
        map->ability.clear();
        map->range.clear();
        map->adjacent.clear();
        vector<position> interactible = InteractibleFrom(*map, cursor->pos,
                                             cursor->selected->min_range, cursor->selected->max_range);

        // Update unit menu with available actions
        *menu = Menu({});

        if(map->tiles[cursor->pos.col][cursor->pos.row].type == GOAL &&
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
        // for ability
        switch(cursor->selected->ability)
        {
            case ABILITY_NONE:
            {
                //cout << "AbilityCommand: You have no ability.\n";
            } break;
            case ABILITY_HEAL:
            {
                for(const position &p : interactible)
                {
                    if(map->tiles[p.col][p.row].occupant &&
                       map->tiles[p.col][p.row].occupant->is_ally &&
                       map->tiles[p.col][p.row].occupant->health < map->tiles[p.col][p.row].occupant->max_health)
                    {
                        map->ability.push_back(p);
                    }
                }
                if(map->ability.size() > 0)
                    menu->AddOption("Heal");
            } break;
            case ABILITY_BUFF:
            {
                for(const position &p : interactible)
                {
                    if(map->tiles[p.col][p.row].occupant &&
                       map->tiles[p.col][p.row].occupant->is_ally)
                    {
                        map->ability.push_back(p);
                    }
                }
                if(map->ability.size() > 0)
                    menu->AddOption("Buff");
            } break;
            case ABILITY_SHIELD:
            {
                cout << "Unimplemented Ability: " << GetAbilityString(cursor->selected->ability) << "\n";
            } break;
            case ABILITY_DANCE:
            {
                for(const position &p : interactible)
                {
                    if(map->tiles[p.col][p.row].occupant &&
                       map->tiles[p.col][p.row].occupant->is_ally &&
                       map->tiles[p.col][p.row].occupant->is_exhausted)
                    {
                        map->ability.push_back(p);
                    }
                }
                if(map->ability.size() > 0)
                    menu->AddOption("Dance");
            } break;
            default:
            {
                assert(!"ERROR Unimplemented Ability in AbilityCommand!\n");
            }
        }

        interactible = InteractibleFrom(*map, cursor->pos, 1, 1);
        // for talking
        for(const position &p : interactible)
        {
            if(map->tiles[p.col][p.row].occupant &&
               map->tiles[p.col][p.row].occupant->is_ally)
            {
                for(const Conversation &conv : conversations->mid_battle)
                {
                    if(((cursor->selected->ID() == conv.one->ID() &&
                         map->tiles[p.col][p.row].occupant->ID() == conv.two->ID())
                            ||
                        (cursor->selected->ID() == conv.two->ID() &&
                         map->tiles[p.col][p.row].occupant->ID() == conv.one->ID()))
                            &&
                        !conv.done
                      )
                    {
                        map->adjacent.push_back(p);
                    }
                }
            }
        }
        if(map->adjacent.size() > 0)
            menu->AddOption("Talk");

        menu->AddOption("Wait");

        EmitEvent(PLACE_UNIT_EVENT);

        // change state
        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    Menu *menu;
    ConversationList *conversations;
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

        cursor->PlaceAt(cursor->redo);

        cursor->path_draw = {};

        cursor->selected->pos = cursor->pos;
        cursor->selected->sheet.ChangeTrack(0);

        EmitEvent(PICK_UP_UNIT_EVENT);

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
        cursor->PlaceAt(next);
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
};

class NextAbilityTargetCommand : public Command
{
public:
    NextAbilityTargetCommand(Cursor *cursor_in, Tilemap *map_in, bool forward_in)
    : cursor(cursor_in),
      map(map_in),
      forward(forward_in)
    {}

    virtual void Execute()
    {
        assert(map->ability.size() > 0);
        if(map->ability.size() == 1)
            return;

        if(forward)
            rotate(map->ability.begin(), 
                   map->ability.begin() + 1,
                   map->ability.end());
        else
            rotate(map->ability.begin(), 
                   map->ability.begin() + map->ability.size() - 1,
                   map->ability.end());
        position next = map->ability[0];

        // move cursor
        cursor->PlaceAt(next);
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
};

class NextTalkTargetCommand : public Command
{
public:
    NextTalkTargetCommand(Cursor *cursor_in, Tilemap *map_in, bool forward_in)
    : cursor(cursor_in),
      map(map_in),
      forward(forward_in)
    {}

    virtual void Execute()
    {
        assert(map->adjacent.size() > 0);
        if(map->adjacent.size() == 1)
            return;

        if(forward)
            rotate(map->adjacent.begin(), 
                   map->adjacent.begin() + 1,
                   map->adjacent.end());
        else
            rotate(map->adjacent.begin(), 
                   map->adjacent.begin() + map->adjacent.size() - 1,
                   map->adjacent.end());
        position next = map->adjacent[0];

        // move cursor
        cursor->PlaceAt(next);
    }

private:
    Cursor *cursor; 
    Tilemap *map;
    bool forward;
};

class DetargetCommand : public Command
{
public:
    DetargetCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    { 
        cursor->PlaceAt(cursor->source);

        GlobalInterfaceState = UNIT_MENU_ROOT;
    }

private:
    Cursor *cursor;
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

class InitiateAbilityCommand : public Command
{
public:
    InitiateAbilityCommand(Cursor *cursor_in, const Tilemap &map_in)
    : cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->pos.col][cursor->pos.row].occupant;

        GlobalInterfaceState = PREVIEW_ABILITY;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
};

class InitiateConversationCommand : public Command
{
public:
    InitiateConversationCommand(ConversationList *conversations_in,
                                Cursor *cursor_in, const Tilemap &map_in)
    : conversations(conversations_in),
      cursor(cursor_in),
      map(map_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = map.tiles[cursor->pos.col][cursor->pos.row].occupant;
        for(Conversation &conv : conversations->mid_battle)
        {
            if((cursor->selected->ID() == conv.one->ID() &&
                cursor->targeted->ID() == conv.two->ID())
                    ||
               (cursor->selected->ID() == conv.two->ID() &&
                cursor->targeted->ID() == conv.one->ID()))
            {
                conversations->current = &conv;
            }
        }

        GlobalInterfaceState = BATTLE_CONVERSATION;
    }

private:
    ConversationList *conversations;
    Cursor *cursor;
    const Tilemap &map;
};

class AttackCommand : public Command
{
public:
    AttackCommand(Cursor *cursor_in, const Tilemap &map_in,
                  Fight *fight_in)
    : cursor(cursor_in),
      map(map_in),
      fight(fight_in)
    {}

    virtual void Execute()
    {
        int distance = ManhattanDistance(cursor->source, cursor->pos);
        direction dir = GetDirection(cursor->source,
                                     cursor->pos);
        *fight = Fight(cursor->selected, cursor->targeted,
                        map.tiles[cursor->source.col][cursor->source.row].avoid,
                        map.tiles[cursor->pos.col][cursor->pos.row].avoid,
                        map.tiles[cursor->source.col][cursor->source.row].defense,
                        map.tiles[cursor->pos.col][cursor->pos.row].defense,
                        distance, dir);
        fight->ready = true;

        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->PlaceAt(cursor->source);
        cursor->path_draw = {};

        GlobalInterfaceState = PLAYER_FIGHT;
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    Fight *fight;
};


class AbilityCommand : public Command
{
public:
    AbilityCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        switch(cursor->selected->ability)
        {
            case ABILITY_NONE:
            {
                cout << "WARN AbilityCommand: How did you get here? You have no ability.\n";
            } break;
            case ABILITY_HEAL:
            {
                SimulateHealing(cursor->selected, cursor->targeted);
                EmitEvent(HEAL_EVENT);
            } break;
            case ABILITY_BUFF:
            {
                cursor->targeted->ApplyBuff(new Buff(STAT_ATTACK, 10, 1));
                EmitEvent(BUFF_EVENT);
            } break;
            case ABILITY_SHIELD:
            {
                cout << "Unimplemented Ability\n";
            } break;
            case ABILITY_DANCE:
            {
                SimulateDancing(cursor->selected, cursor->targeted);
                EmitEvent(DANCE_EVENT);
            } break;
            default:
            {
                assert(!"ERROR Unimplemented Ability in AbilityCommand!\n");
            }
        }

        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->PlaceAt(cursor->source);
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

class BackDownFromAbilityCommand : public Command
{
public:
    BackDownFromAbilityCommand(Cursor *cursor_in)
    : cursor(cursor_in)
    {}

    virtual void Execute()
    {
        cursor->targeted = nullptr;

        GlobalInterfaceState = ABILITY_TARGETING;
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
        EmitEvent(UNIT_INFO_EVENT);
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
        cursor->PlaceAt(cursor->redo);
        cursor->selected = nullptr;
        cursor->redo = {-1, -1};

        GlobalInterfaceState = NEUTRAL_OVER_ENEMY;
        EmitEvent(PLACE_UNIT_EVENT);
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
        EmitEvent(SELECT_MENU_OPTION_EVENT);
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
        EmitEvent(SELECT_MENU_OPTION_EVENT);

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

            cursor->PlaceAt(map.attackable[0]);
            GlobalInterfaceState = ATTACK_TARGETING;
            return;
        }
        if(option == "Heal" || option == "Dance" ||
           option == "Buff")
        {
            assert(map.ability.size());
            cursor->source = cursor->pos;

            cursor->PlaceAt(map.ability[0]);
            GlobalInterfaceState = ABILITY_TARGETING;
            return;
        }
        if(option == "Talk")
        {
            assert(map.adjacent.size());
            cursor->source = cursor->pos;

            cursor->PlaceAt(map.adjacent[0]);
            GlobalInterfaceState = TALK_TARGETING;
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
        EmitEvent(UNIT_INFO_EVENT);
    }
};

class BackOutOfUnitInfoCommand : public Command
{
public:
    virtual void Execute()
    {
        GlobalInterfaceState = NEUTRAL_OVER_UNIT;
        EmitEvent(PLACE_UNIT_EVENT);
    }
};

class ChooseLevelMenuOptionCommand : public Command
{
public:
    ChooseLevelMenuOptionCommand(const Menu &menu_in,
                                 Menu *conversation_menu_in,
                                 ConversationList *conversations_in)
    : menu(menu_in),
      conversation_menu(conversation_menu_in),
      conversations(conversations_in)
    {}

    virtual void Execute()
    {
        EmitEvent(SELECT_MENU_OPTION_EVENT);

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
            *conversation_menu = Menu({});
            char buffer[256] = "";

            for(const Conversation &conv : conversations->list)
            {
                sprintf(buffer, "%s / %s", conv.one->name.c_str(), conv.two->name.c_str());
                conversation_menu->AddOption(buffer);
            }
            conversation_menu->AddOption("Return");

            GlobalInterfaceState = CONVERSATION_MENU;
            return;
        }

        assert(!"ERROR ChooseLevelMenuOptionCommand | How did you get here?\n");
    }

private:
    const Menu &menu;
    Menu *conversation_menu;
    ConversationList *conversations;
};


class ReturnToLevelMenuCommand : public Command
{
public:
    ReturnToLevelMenuCommand()
    {}

    virtual void Execute()
    {
        EmitEvent(SELECT_MENU_OPTION_EVENT);
        GlobalInterfaceState = LEVEL_MENU;
    }

private:
};


class ChooseConversationMenuOptionCommand : public Command
{
public:
    ChooseConversationMenuOptionCommand(const Menu &menu_in, ConversationList *conversations_in)
    : menu(menu_in),
      conversations(conversations_in)
    {}

    virtual void Execute()
    {
        EmitEvent(SELECT_MENU_OPTION_EVENT);

        string option = menu.optionText[menu.current];

        if(menu.current < conversations->list.size())
        {
            if(conversations->list[menu.current].done)
                return;

            conversations->index = menu.current;
            GlobalInterfaceState = CONVERSATION;
            return;
        }
        if(option == "Return")
        {
            GlobalInterfaceState = LEVEL_MENU;
            return;
        }
        else
        {
            cout << "UNIMPLEMENTED\n";
            return;
        }

        assert(!"ERROR ChooseConversationMenuOptionCommand | How did you get here?\n");
    }

private:
    const Menu &menu;
    ConversationList *conversations;
};


// ================================== Conversations ============================
class NextSentenceCommand : public Command
{
public:
    NextSentenceCommand(Cursor *cursor_in, Conversation *conversation_in)
    : cursor(cursor_in),
      conversation(conversation_in)
    {}

    virtual void Execute()
    {
        conversation->Next();
        if(conversation->done)
        {
            if(GlobalInterfaceState == PRELUDE)
            {
                GlobalInterfaceState = NEUTRAL_OVER_UNIT;
                return;
            }
            if(GlobalInterfaceState == BATTLE_CONVERSATION)
            {
                cursor->pos = cursor->selected->pos;
                cursor->selected->Deactivate();
                GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
                return;
            }
            if(GlobalInterfaceState == CONVERSATION)
            {
                GlobalInterfaceState = CONVERSATION_MENU;
                return;
            }
        }

        conversation->ReloadTextures();
        EmitEvent(NEXT_SENTENCE_EVENT);
    }

private:
    Cursor *cursor;
    Conversation *conversation;
};

class EndConversationEarlyCommand : public Command
{
public:
    EndConversationEarlyCommand(Cursor *cursor_in, Conversation *conversation_in)
    : cursor(cursor_in),
      conversation(conversation_in)
    {}

    virtual void Execute()
    {
        cout << conversation->filename << "\n";
        conversation->done = true;

        if(GlobalInterfaceState == PRELUDE)
        {
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;
            return;
        }
        if(GlobalInterfaceState == BATTLE_CONVERSATION)
        {
            cursor->pos = cursor->selected->pos;
            cursor->selected->Deactivate();
            GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
            return;
        }
        if(GlobalInterfaceState == CONVERSATION)
        {
            GlobalInterfaceState = CONVERSATION_MENU;
            return;
        }
    }

private:
    Cursor *cursor;
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
    void UpdateCommands(Cursor *cursor, Tilemap *map,
                        Menu *gameMenu, Menu *unitMenu,
                        Menu *levelMenu, Menu *conversationMenu, 
                        ConversationList *conversations,
                        Fight *fight)
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
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<CycleUnitsCommand>(cursor, map, true));
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
                BindL(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(SELECTED_OVER_GROUND):
            {
                BindUp(make_shared<MoveSCommand>(cursor, *map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, *map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, *map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, *map, direction(1, 0)));
                BindA(make_shared<PlaceUnitCommand>(cursor, map, unitMenu, conversations));
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(ATTACK_TARGETING):
            {
                BindUp(make_shared<NextAttackTargetCommand>(cursor, map, true));
                BindDown(make_shared<NextAttackTargetCommand>(cursor, map, false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateAttackCommand>(cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;
            case(ABILITY_TARGETING):
            {
                BindUp(make_shared<NextAbilityTargetCommand>(cursor, map, true));
                BindDown(make_shared<NextAbilityTargetCommand>(cursor, map, false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateAbilityCommand>(cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(TALK_TARGETING):
            {
                BindUp(make_shared<NextTalkTargetCommand>(cursor, map, true));
                BindDown(make_shared<NextTalkTargetCommand>(cursor, map, false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateConversationCommand>(conversations, cursor, *map));
                BindB(make_shared<DetargetCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(PREVIEW_ATTACK):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<AttackCommand>(cursor, *map, fight));
                BindB(make_shared<BackDownFromAttackingCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(PREVIEW_ABILITY):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<AbilityCommand>(cursor));
                BindB(make_shared<BackDownFromAbilityCommand>(cursor));
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
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

            case(UNIT_MENU_ROOT):
            {
                BindUp(make_shared<UpdateMenuCommand>(unitMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(unitMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseUnitMenuOptionCommand>(cursor, *map, *unitMenu));
                BindB(make_shared<UndoPlaceUnitCommand>(cursor, map));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(UNIT_INFO):
            {
                BindUp(make_shared<CycleUnitsCommand>(cursor, map, false));
                BindDown(make_shared<CycleUnitsCommand>(cursor, map, true));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NullCommand>());
                BindB(make_shared<BackOutOfUnitInfoCommand>());
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
                BindR(make_shared<DeselectEnemyCommand>(cursor));
            } break;
            case(ENEMY_RANGE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<EnemyUndoRangeCommand>());
                BindB(make_shared<EnemyUndoRangeCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(LEVEL_MENU):
            {
                BindUp(make_shared<UpdateMenuCommand>(levelMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(levelMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseLevelMenuOptionCommand>(*levelMenu, conversationMenu, conversations));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(CONVERSATION_MENU):
            {
                BindUp(make_shared<UpdateMenuCommand>(conversationMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(conversationMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseConversationMenuOptionCommand>(*conversationMenu, conversations));
                BindB(make_shared<ReturnToLevelMenuCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(CONVERSATION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, &(conversations->list[conversations->index])));
                BindB(make_shared<EndConversationEarlyCommand>(cursor, &(conversations->list[conversations->index])));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(BATTLE_CONVERSATION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, conversations->current));
                BindB(make_shared<EndConversationEarlyCommand>(cursor, conversations->current));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(PRELUDE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, &(conversations->prelude)));
                BindB(make_shared<EndConversationEarlyCommand>(cursor, &(conversations->prelude)));
                BindL(make_shared<NullCommand>());
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
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(PLAYER_FIGHT):
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
    shared_ptr<Command> buttonL;
    shared_ptr<Command> buttonR;

    queue<shared_ptr<Command>> commandQueue = {};
};

#endif
