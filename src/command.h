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
            cursor->MoveTo(new_pos, dir * -1, map.width, map.height);

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

        map->accessible.clear();
        map->vis_range.clear();
        pair<vector<position>, vector<position>> result = 
            AccessibleAndAttackableFrom(*map, cursor->redo,
                                        cursor->selected->movement,
                                        cursor->selected->min_range,
                                        cursor->selected->max_range,
                                        cursor->selected->is_ally);
        map->accessible = result.first;
        map->vis_range = result.second;

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
        cursor->MoveTo(new_pos, dir * -1, map.width, map.height);

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
    PlaceUnitCommand(Cursor *cursor_in, Level *level_in, Menu *menu_in)
    : cursor(cursor_in),
      level(level_in),
      menu(menu_in)
    {}

    virtual void Execute()
    {
        // Determine interactible squares
        level->map.attackable.clear();
        level->map.ability.clear();
        level->map.range.clear();
        level->map.adjacent.clear();

        // Update unit menu with available actions
        *menu = Menu({});

        if(level->map.tiles[cursor->pos.col][cursor->pos.row].type == VILLAGE)
        {
            for(const Conversation &conv : level->conversations.villages)
            {
                if(conv.pos == cursor->pos && !conv.done)
                    menu->AddOption("Visit");
            }
        }

        if(level->objective == OBJECTIVE_CAPTURE &&
           level->map.tiles[cursor->pos.col][cursor->pos.row].type == GOAL &&
           cursor->selected->ID() == LEADER_ID)
        {
            // TODO: Don't allow this option at all if the level isn't a capture objective.
            menu->AddOption("Capture");
        }

        vector<position> interactible = InteractibleFrom(level->map, cursor->pos,
                                             cursor->selected->min_range, cursor->selected->max_range);
        // for attacking
        for(const position &p : interactible)
        {
            level->map.range.push_back(p);
            if(level->map.tiles[p.col][p.row].occupant &&
               !level->map.tiles[p.col][p.row].occupant->is_ally)
            {
                level->map.attackable.push_back(p);
            }
        }
        if(level->map.attackable.size() > 0)
            menu->AddOption("Attack");

        interactible = InteractibleFrom(level->map, cursor->pos, 1, 1);
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
                    if(level->map.tiles[p.col][p.row].occupant &&
                       level->map.tiles[p.col][p.row].occupant->is_ally &&
                       level->map.tiles[p.col][p.row].occupant->health < level->map.tiles[p.col][p.row].occupant->max_health &&
                       level->map.tiles[p.col][p.row].occupant->ID() != cursor->selected->ID())
                    {
                        level->map.ability.push_back(p);
                    }
                }
                if(level->map.ability.size() > 0)
                    menu->AddOption("Heal");
            } break;
            case ABILITY_BUFF:
            {
                for(const position &p : interactible)
                {
                    if(level->map.tiles[p.col][p.row].occupant &&
                       level->map.tiles[p.col][p.row].occupant->is_ally &&
                       level->map.tiles[p.col][p.row].occupant->ID() != cursor->selected->ID())
                    {
                        level->map.ability.push_back(p);
                    }
                }
                if(level->map.ability.size() > 0)
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
                    if(level->map.tiles[p.col][p.row].occupant &&
                       level->map.tiles[p.col][p.row].occupant->is_ally &&
                       level->map.tiles[p.col][p.row].occupant->is_exhausted)
                    {
                        level->map.ability.push_back(p);
                    }
                }
                if(level->map.ability.size() > 0)
                    menu->AddOption("Dance");
            } break;
            default:
            {
                SDL_assert(!"ERROR Unimplemented Ability in AbilityCommand!\n");
            }
        }

        interactible = InteractibleFrom(level->map, cursor->pos, 1, 1);
        // for talking
        for(const position &p : interactible)
        {
            if(level->map.tiles[p.col][p.row].occupant &&
               level->map.tiles[p.col][p.row].occupant->is_ally)
            {
                for(const Conversation &conv : level->conversations.mid_battle)
                {
                    if(((cursor->selected->ID() == conv.one->ID() &&
                         level->map.tiles[p.col][p.row].occupant->ID() == conv.two->ID())
                            ||
                        (cursor->selected->ID() == conv.two->ID() &&
                         level->map.tiles[p.col][p.row].occupant->ID() == conv.one->ID()))
                            &&
                        !conv.done
                      )
                    {
                        level->map.adjacent.push_back(p);
                    }
                }
            }
        }
        if(level->map.adjacent.size() > 0)
            menu->AddOption("Talk");

        menu->AddOption("Wait");

        if(cursor->path_draw.empty())
        {
            GlobalInterfaceState = UNIT_MENU_ROOT;
            level->map.tiles[cursor->redo.col][cursor->redo.row].occupant = nullptr;
            level->map.tiles[cursor->pos.col][cursor->pos.row].occupant = cursor->selected;

            cursor->selected->pos = cursor->pos;
            cursor->selected->sheet.ChangeTrack(TRACK_ACTIVE);
            EmitEvent(PLACE_UNIT_EVENT);
            return;
        }

        cursor->unit_animation = GetAnimation(MOVE_UNIT_ANIMATION, cursor->path_draw.size());

        // change state
        GlobalInterfaceState = ANIMATING_UNIT_MOVEMENT;
    }

private:
    Cursor *cursor; 
    Menu *menu;
    Level *level;
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
        cursor->selected->sheet.ChangeTrack(TRACK_IDLE);

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
        SDL_assert(map->attackable.size() > 0);
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
        SDL_assert(map->ability.size() > 0);
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
        SDL_assert(map->adjacent.size() > 0);
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
                                Cursor *cursor_in, const Tilemap &map_in,
                                Sound *level_song_in)
    : conversations(conversations_in),
      cursor(cursor_in),
      map(map_in),
      level_song(level_song_in)
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
                conversations->current_conversation = &conv;
                if(conv.song)
                {
                    level_song->Pause();
                    conv.song->Start();
                }
            }
        }

        GlobalInterfaceState = BATTLE_CONVERSATION;
    }

private:
    ConversationList *conversations;
    Cursor *cursor;
    const Tilemap &map;
    Sound *level_song;
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

                int experience = EXP_FOR_HEALING;
                EmitEvent(Event(EXPERIENCE_EVENT, cursor->selected, experience, (float)experience / 10.0f + 0.3f));
            } break;
            case ABILITY_BUFF:
            {
                SimulateBuff(cursor->selected, cursor->targeted);
                EmitEvent(BUFF_EVENT);

                int experience = EXP_FOR_BUFF;
                EmitEvent(Event(EXPERIENCE_EVENT, cursor->selected, experience, (float)experience / 10.0f + 0.3f));
            } break;
            case ABILITY_SHIELD:
            {
                cout << "Unimplemented Ability\n";
            } break;
            case ABILITY_DANCE:
            {
                SimulateDancing(cursor->selected, cursor->targeted);
                EmitEvent(DANCE_EVENT);

                int experience = EXP_FOR_DANCE;
                EmitEvent(Event(EXPERIENCE_EVENT, cursor->selected, experience, (float)experience / 10.0f + 0.3f));
                GlobalAIState = AI_RESOLVING_EXPERIENCE;
            } break;
            default:
            {
                SDL_assert(!"ERROR Unimplemented Ability in AbilityCommand!\n");
            }
        }

        cursor->selected->Deactivate();
        cursor->selected = nullptr;
        cursor->targeted = nullptr;
        cursor->PlaceAt(cursor->source);
        cursor->path_draw = {};

        GlobalInterfaceState = RESOLVING_EXPERIENCE;
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

        map->accessible.clear();
        map->vis_range.clear();
        pair<vector<position>, vector<position>> result = 
            AccessibleAndAttackableFrom(*map, cursor->redo,
                                        cursor->selected->movement,
                                        cursor->selected->min_range,
                                        cursor->selected->max_range,
                                        cursor->selected->is_ally);
        map->accessible = result.first;
        map->vis_range = result.second;

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
    ChooseGameMenuOptionCommand(Menu *menu_in, Level *level_in)
    : menu(menu_in),
      level(level_in)
    {}

    virtual void Execute()
    { 
        EmitEvent(SELECT_MENU_OPTION_EVENT);
        switch(menu->current)
        {
            case(0): // OUTLOOK
            {
                GlobalInterfaceState = GAME_MENU_OUTLOOK;
                return;
            } break;
            case(1): // OPTIONS
            {
                GlobalInterfaceState = GAME_MENU_OPTIONS;
                return;
            } break;
            case(2): // END TURN
            {
                for(cutscene &cs : level->conversations.cutscenes)
                {
                    if(cs.first == level->turn_count)
                    {
                        level->conversations.current_cutscene = &cs;
                        GlobalInterfaceState = CUTSCENE;
                        return;
                    }
                }

                GlobalPlayerTurn = false;
                GlobalInterfaceState = NO_OP;
                EmitEvent(START_AI_TURN_EVENT);
                GlobalAIState = AI_NO_OP;
                return;
            } break;
        }
    }

private:
    Menu *menu;
    Level *level;
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
    ChooseUnitMenuOptionCommand(Cursor *cursor_in, const Tilemap &map_in, const Menu &menu_in,
                                Sound *level_song_in, ConversationList *conversations_in)
    : cursor(cursor_in),
      map(map_in),
      menu(menu_in),
      level_song(level_song_in),
      conversations(conversations_in)
    {}

    virtual void Execute()
    {
        EmitEvent(SELECT_MENU_OPTION_EVENT);

        string option = menu.optionText[menu.current];

        if(option == "Visit")
        {
            for(Conversation &conv : conversations->villages)
            {
                if(conv.pos == cursor->pos)
                {
                    conversations->current_conversation = &conv;
                }
            }
            assert(conversations->current_conversation);
            conversations->current_conversation->one = cursor->selected;
            conversations->current_conversation->ReloadTextures(); // NOTE: For name to update

            if(conversations->current_conversation->song)
            {
                level_song->Pause();
                conversations->current_conversation->song->Start();
            }

            // Move onto next level!
            GlobalInterfaceState = VILLAGE_CONVERSATION;
            return;
        }

        if(option == "Capture")
        {
            cursor->selected->Deactivate();

            cursor->selected = nullptr;
            cursor->redo = {-1, -1};
            cursor->path_draw = {};

            level_song->FadeOut();
            GlobalInterfaceState = LEVEL_MENU;
            EmitEvent(MISSION_COMPLETE_EVENT);
            return;
        }

        if(option == "Attack")
        {
            SDL_assert(map.attackable.size());
            cursor->source = cursor->pos;

            cursor->PlaceAt(map.attackable[0]);
            GlobalInterfaceState = ATTACK_TARGETING;
            return;
        }
        if(option == "Heal" || option == "Dance" ||
           option == "Buff")
        {
            SDL_assert(map.ability.size());
            cursor->source = cursor->pos;

            cursor->PlaceAt(map.ability[0]);
            GlobalInterfaceState = ABILITY_TARGETING;
            return;
        }
        if(option == "Talk")
        {
            SDL_assert(map.adjacent.size());
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

        SDL_assert(!"ERROR ChooseUnitMenuOptionCommand | How did you get here?\n");
    }

private:
    Cursor *cursor;
    const Tilemap &map;
    const Menu &menu;
    Sound *level_song;
    ConversationList *conversations;
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
                                 Level *level_in,
                                 const vector<shared_ptr<Unit>> &units_in,
                                 const vector<shared_ptr<Unit>> &party_in,
                                 Menu *conversation_menu_in,
                                 ConversationList *conversations_in)
    : menu(menu_in),
      level(level_in),
      units(units_in),
      party(party_in),
      conversation_menu(conversation_menu_in),
      conversations(conversations_in)
    {}

    virtual void Execute()
    {
        EmitEvent(SELECT_MENU_OPTION_EVENT);

        string option = menu.optionText[menu.current];

        if(option == "Next")
        {
            level->next_level = true;
            level->turn_start = true;
            EmitEvent(NEXT_LEVEL_EVENT);
            GlobalInterfaceState = PRELUDE;
            return;
        }
        if(option == "Redo")
        {
            *level = LoadLevel(level->name, units, party);
            level->song->Restart();

            GlobalPlayerTurn = true;
            level->turn_start = true;
            EmitEvent(START_PLAYER_TURN_EVENT);
            GlobalInterfaceState = NO_OP;
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

        SDL_assert(!"ERROR ChooseLevelMenuOptionCommand | How did you get here?\n");
    }

private:
    const Menu &menu;
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
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
    ChooseConversationMenuOptionCommand(const Menu &menu_in, ConversationList *conversations_in,
                                        Sound *level_song_in)
    : menu(menu_in),
      conversations(conversations_in),
      level_song(level_song_in)
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

            if(conversations->list[menu.current].song)
            {
                level_song->Pause();
                conversations->list[menu.current].song->Start();
            }
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

        SDL_assert(!"ERROR ChooseConversationMenuOptionCommand | How did you get here?\n");
    }

private:
    const Menu &menu;
    ConversationList *conversations;
    Sound *level_song;
};


// ================================== Conversations ============================
class NextSentenceCommand : public Command
{
public:
    NextSentenceCommand(Cursor *cursor_in, Conversation *conversation_in,
                        Sound *level_song_in, bool end_in)
    : cursor(cursor_in),
      conversation(conversation_in),
      level_song(level_song_in),
      end(end_in)
    {}

    virtual void Execute()
    {
        conversation->Next();
        if(!end && !conversation->done)
        {
            conversation->ReloadTextures();
            EmitEvent(NEXT_SENTENCE_EVENT);
            return;
        }

        conversation->done = true;
        if(GlobalInterfaceState == PRELUDE)
        {
            conversation->song->Stop();
            level_song->Start();

            EmitEvent(START_PLAYER_TURN_EVENT);
            GlobalInterfaceState = NO_OP;
            return;
        }
        if(GlobalInterfaceState == CUTSCENE)
        {
            conversation->song->Stop();
            level_song->Start();

            GlobalPlayerTurn = false;
            EmitEvent(START_AI_TURN_EVENT);
            GlobalInterfaceState = NO_OP;
            GlobalAIState = AI_NO_OP;
            return;
        }
        if(GlobalInterfaceState == BATTLE_CONVERSATION)
        {
            conversation->song->Stop();
            level_song->Start();

            cursor->pos = cursor->selected->pos;
            cursor->selected->Deactivate();
            cursor->selected = nullptr;
            cursor->targeted = nullptr;
            cursor->path_draw = {};
            GlobalInterfaceState = NEUTRAL_OVER_DEACTIVATED_UNIT;
            return;
        }
        if(GlobalInterfaceState == VILLAGE_CONVERSATION)
        {
            conversation->song->Stop();
            level_song->Start();

            cursor->selected->Deactivate();
            int experience = EXP_FOR_VILLAGE_SAVED;
            cursor->selected->GrantExperience(experience);
            EmitEvent(Event(EXPERIENCE_EVENT, cursor->selected, experience));
            cursor->selected = nullptr;
            cursor->targeted = nullptr;
            cursor->path_draw = {};
            GlobalInterfaceState = RESOLVING_EXPERIENCE;
            return;
        }
        if(GlobalInterfaceState == CONVERSATION)
        {
            conversation->song->FadeOut();

            GlobalInterfaceState = CONVERSATION_MENU;
            return;
        }
    }

private:
    Cursor *cursor;
    Conversation *conversation;
    Sound *level_song;
    bool end;
};

// ================================= Game Over =================================
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

        GlobalPlayerTurn = true;
        level->turn_start = true;
        EmitEvent(START_PLAYER_TURN_EVENT);
        GlobalInterfaceState = NO_OP;
    }

private:
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
};

class StartGameCommand : public Command
{
public:
    StartGameCommand(Level *level_in,
                     const vector<shared_ptr<Unit>> &units_in,
                     const vector<shared_ptr<Unit>> &party_in
                     )
    : level(level_in),
      units(units_in),
      party(party_in)
    {}

    virtual void Execute()
    {
        GlobalPlayerTurn = true;
        *level = LoadLevel(level->name, units, party);
        level->turn_start = true;
        level->conversations.prelude.song->Start();

        EmitEvent(START_GAME_EVENT);
        GlobalInterfaceState = PRELUDE;
        return;
    }

private:
    Level *level;
    const vector<shared_ptr<Unit>> &units;
    const vector<shared_ptr<Unit>> &party;
};

class SayGoodbyeEvent : public Command
{
public:
    virtual void Execute()
    {
        EmitEvent(UNIT_DEATH_OVER_EVENT);
        return;
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
        // NOTE: uncomment if the player doesn't have any inputs on AI turn.
        //if(!GlobalPlayerTurn)
        //    return;

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
    void UpdateCommands(Cursor *cursor, Level *level, 
                        const vector<shared_ptr<Unit>> &units,
                        const vector<shared_ptr<Unit>> &party,
                        Menu *gameMenu, Menu *unitMenu,
                        Menu *levelMenu, Menu *conversationMenu, 
                        Fight *fight)
    {
        // NOTE: uncomment if the player doesn't have any inputs on AI turn.
        //if(!GlobalPlayerTurn)
        //    return;

        switch(GlobalInterfaceState)
        {
            case(NEUTRAL_OVER_GROUND):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(NEUTRAL_OVER_ENEMY):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<EnemyRangeCommand>(cursor, &(level->map)));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<SelectEnemyCommand>(cursor, &(level->map)));
            } break;

            case(NEUTRAL_OVER_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<SelectUnitCommand>(cursor, &(level->map)));
                BindB(make_shared<NullCommand>());
                BindL(make_shared<CycleUnitsCommand>(cursor, &(level->map), true));
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(NEUTRAL_OVER_DEACTIVATED_UNIT):
            {
                BindUp(make_shared<MoveCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<OpenGameMenuCommand>());
                BindB(make_shared<NullCommand>());
                BindL(make_shared<NullCommand>());
                BindR(make_shared<OpenUnitInfoCommand>());
            } break;

            case(SELECTED_OVER_GROUND):
            {
                BindUp(make_shared<MoveSCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<PlaceUnitCommand>(cursor, level, unitMenu));
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_INACCESSIBLE):
            {
                BindUp(make_shared<MoveSCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<NullCommand>());
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ALLY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<NullCommand>()); // CONSIDER: Move and Heal?
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(SELECTED_OVER_ENEMY):
            {
                BindUp(make_shared<MoveSCommand>(cursor, level->map, direction(0, -1)));
                BindDown(make_shared<MoveSCommand>(cursor, level->map, direction(0, 1)));
                BindLeft(make_shared<MoveSCommand>(cursor, level->map, direction(-1, 0)));
                BindRight(make_shared<MoveSCommand>(cursor, level->map, direction(1, 0)));
                BindA(make_shared<NullCommand>()); // CONSIDER: Move and Attack?
                BindB(make_shared<DeselectUnitCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(ATTACK_TARGETING):
            {
                BindUp(make_shared<NextAttackTargetCommand>(cursor, &(level->map), true));
                BindDown(make_shared<NextAttackTargetCommand>(cursor, &(level->map), false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateAttackCommand>(cursor, level->map));
                BindB(make_shared<DetargetCommand>(cursor));
                BindR(make_shared<NullCommand>());
            } break;
            case(ABILITY_TARGETING):
            {
                BindUp(make_shared<NextAbilityTargetCommand>(cursor, &(level->map), true));
                BindDown(make_shared<NextAbilityTargetCommand>(cursor, &(level->map), false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateAbilityCommand>(cursor, level->map));
                BindB(make_shared<DetargetCommand>(cursor));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(TALK_TARGETING):
            {
                BindUp(make_shared<NextTalkTargetCommand>(cursor, &(level->map), true));
                BindDown(make_shared<NextTalkTargetCommand>(cursor, &(level->map), false));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<InitiateConversationCommand>(&(level->conversations), cursor, level->map, level->song));
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
                BindA(make_shared<AttackCommand>(cursor, level->map, fight));
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
                BindA(make_shared<ChooseGameMenuOptionCommand>(gameMenu, level));
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

            case(ANIMATING_UNIT_MOVEMENT):
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

            case(UNIT_MENU_ROOT):
            {
                BindUp(make_shared<UpdateMenuCommand>(unitMenu, -1));
                BindDown(make_shared<UpdateMenuCommand>(unitMenu, 1));
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<ChooseUnitMenuOptionCommand>(cursor, level->map, *unitMenu, level->song, &(level->conversations)));
                BindB(make_shared<UndoPlaceUnitCommand>(cursor, &(level->map)));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;
            case(UNIT_INFO):
            {
                BindUp(make_shared<CycleUnitsCommand>(cursor, &(level->map), false));
                BindDown(make_shared<CycleUnitsCommand>(cursor, &(level->map), true));
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
                BindA(make_shared<ChooseLevelMenuOptionCommand>(*levelMenu, level, units, party, conversationMenu, &(level->conversations)));
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
                BindA(make_shared<ChooseConversationMenuOptionCommand>(*conversationMenu, &(level->conversations), level->song));
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
                BindA(make_shared<NextSentenceCommand>(cursor, &(level->conversations.list[level->conversations.index]), level->song, false));
                BindB(make_shared<NextSentenceCommand>(cursor, &(level->conversations.list[level->conversations.index]), level->song, true));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(BATTLE_CONVERSATION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, level->conversations.current_conversation, level->song, false));
                BindB(make_shared<NextSentenceCommand>(cursor, level->conversations.current_conversation, level->song, true));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(VILLAGE_CONVERSATION):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, level->conversations.current_conversation, level->song, false));
                BindB(make_shared<NextSentenceCommand>(cursor, level->conversations.current_conversation, level->song, true));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(PRELUDE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, &(level->conversations.prelude), level->song, false));
                BindB(make_shared<NextSentenceCommand>(cursor, &(level->conversations.prelude), level->song, true));
                BindL(make_shared<NullCommand>());
                BindR(make_shared<NullCommand>());
            } break;

            case(CUTSCENE):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<NextSentenceCommand>(cursor, &(level->conversations.current_cutscene->second), level->song, false));
                BindB(make_shared<NextSentenceCommand>(cursor, &(level->conversations.current_cutscene->second), level->song, true));
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

            case(TITLE_SCREEN):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<StartGameCommand>(level, units, party));
                BindB(make_shared<NullCommand>());
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

            case(RESOLVING_EXPERIENCE):
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

            case(RESOLVING_ADVANCEMENT):
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

            case(DEATH):
            {
                BindUp(make_shared<NullCommand>());
                BindDown(make_shared<NullCommand>());
                BindLeft(make_shared<NullCommand>());
                BindRight(make_shared<NullCommand>());
                BindA(make_shared<SayGoodbyeEvent>());
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
