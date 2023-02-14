// Author: Alex Hartford
// Program: Emblem
// File: Structs

#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>

// =============================== small-time ===================================
struct InputState
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
    bool l;
    bool r;

	int joystickCooldown = 0;
};

// ================================= Rendering =================================
struct Texture
{
    SDL_Texture *sdl_texture;
    string filename;
    string dir;
    int width;
    int height;

    Texture(SDL_Texture *sdl_texture_in, string dir_in, string filename_in, int width_in, int height_in)
    {
        this->sdl_texture = sdl_texture_in;
        this->width = width_in;
        this->height = height_in;
        this->dir = dir_in;
        this->filename = filename_in;
    }

    Texture()
    {
        //printf("WARN: Default texture constructor called.\n");
    }

    ~Texture()
    {
        //TODO: This causes some funny stuff to happen!
        //      But I think it has to be called. Look into this.
        //SDL_DestroyTexture(sdl_texture);
    }
};


enum SheetTrack
{
    TRACK_IDLE,
    TRACK_ACTIVE,
    TRACK_LEFT,
    TRACK_RIGHT,
    TRACK_DOWN,
    TRACK_UP,
};

struct Spritesheet
{
    Texture texture;
    int size    = SPRITE_SIZE;
    int tracks  = 0;
    int frames  = 0;
    SheetTrack track = TRACK_IDLE;
    int frame   = 0;
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;

    Spritesheet(Texture texture_in, int size_in, int speed_in)
    : texture(texture_in),
      size(size_in),
      speed(speed_in)
    {
        this->tracks = texture_in.height / size_in;
        this->frames = texture_in.width / size_in;
    }

    // called each frame
    void
    Update()
    {
        counter++;
        if(!(counter % speed))
        {
            int new_frame = frame + 1;
            if(new_frame >= frames)
                this->frame = 0;
            else
                this->frame = new_frame;
        }
    }

    // switches the sprite to the next animation track
    void
    ChangeTrack(SheetTrack track_in)
    {
        SDL_assert(track_in < tracks && track_in >= 0);
        this->track = track_in;
        this->frame = 0;
    }

    void
    ChangeTrackMovement(const direction &dir)
    {
        if(dir == position(1, 0))
            ChangeTrack(TRACK_RIGHT);
        else if(dir == position(-1, 0))
            ChangeTrack(TRACK_LEFT);
        else if(dir == position(0, -1))
            ChangeTrack(TRACK_DOWN);
        else if(dir == position(0, 1))
            ChangeTrack(TRACK_UP);
        else
            assert(!"ChangeTrackMovement: Invalid direction.");
    }
};

// =================================== Gameplay ================================
enum Ability
{
    ABILITY_NONE,
    ABILITY_HEAL,
    ABILITY_BUFF,
    ABILITY_SHIELD,
    ABILITY_DANCE,
};

enum Stat
{
    STAT_NONE,
    STAT_ATTACK,
    STAT_DEFENSE,
    STAT_APTITUDE,
    STAT_SPEED,
    STAT_SKILL,
};

struct Buff
{
    Stat stat = STAT_NONE;
    int amount = 5;
    int turns_remaining = 1;

    Buff(Stat stat_in, int amount_in, int turns_remaining_in)
    : stat(stat_in),
      amount(amount_in),
      turns_remaining(turns_remaining_in)
    {}
};


enum Expression
{
    EXPR_NEUTRAL,
    EXPR_HAPPY,
    EXPR_ANGRY,
    EXPR_WINCE,
};


struct Unit
{
    string name;
    bool is_ally;
    int health;

    int strength;
    int dexterity;
    int vitality;
    int intuition;
    int faith;

    int level;
    Ability ability;

    int experience = 0;

    Item *weapon;
    Item *pocket;

    int arrival = 0;
    int turns_active = -1;
    int xp_value = 0;
    AIBehavior ai_behavior = NO_BEHAVIOR;
    position pos = {0, 0};
    bool is_exhausted = false;
    bool should_die = false;
    bool has_spoken_valediction = false;
    position animation_offset = {0, 0};
    bool is_boss = false;

    Buff *buff = nullptr;

    Spritesheet sheet;
    Texture neutral;
    Texture happy;
    Texture angry;
    Texture wince;

    string valediction;

    ~Unit()
    {
        delete buff;
        delete weapon;
        delete pocket;
    }

    size_t
    ID()
    {
        return hash<string>{}(name);
    }

    void
    Update()
    {
        sheet.Update();
    }

    Unit(
         string name_in, bool is_ally_in,
         int strength_in, int dexterity_in,
         int vitality_in,
         int intuition_in, int faith_in,
         int level_in,
         Ability ability_in,
         AIBehavior ai_behavior_in,
         int xp_value_in,

         ItemType weapon_type_in,
         ItemType pocket_type_in,

         Spritesheet sheet_in,
         Texture neutral_in,
         Texture happy_in,
         Texture angry_in,
         Texture wince_in,
         const string &valediction_in
         )
    : name(name_in),
      is_ally(is_ally_in),
      strength(strength_in),
      dexterity(dexterity_in),
      vitality(vitality_in),
      intuition(intuition_in),
      faith(faith_in),
      level(level_in),
      ability(ability_in),
      ai_behavior(ai_behavior_in),
      xp_value(xp_value_in),
      sheet(sheet_in),
      neutral(neutral_in),
      happy(happy_in),
      angry(angry_in),
      wince(wince_in),
      valediction(valediction_in)
    {
        health = MaxHealth();
        weapon = GetItem(weapon_type_in);
        pocket = GetItem(pocket_type_in);
    }

    Unit(const Unit &other)
    : name(other.name),
      is_ally(other.is_ally),
      health(other.health),
      strength(other.strength),
      dexterity(other.dexterity),
      vitality(other.vitality),
      intuition(other.intuition),
      faith(other.faith),
      level(other.level),
      ability(other.ability),
      ai_behavior(other.ai_behavior),
      xp_value(other.xp_value),
      sheet(other.sheet),
      neutral(other.neutral),
      happy(other.happy),
      angry(other.angry),
      wince(other.wince)
    {
        if(other.weapon)
            weapon = new Item(*other.weapon);
        if(other.pocket)
            pocket = new Item(*other.pocket);
    }

    // Switches the primary weapon with the secondary item
    void
    SwitchItems()
    {
        assert(weapon->weapon);
        Item *tmp = weapon;
        weapon = pocket;
        pocket = tmp;
    }

    // Uses the pocket item
    void
    Use()
    {
        assert(pocket);
        if(!pocket->consumable)
        {
            cout << "WARNING: Item.Use() not right. Item type: " << GetItemString(pocket->type) << "\n";
            return;
        }

        switch(pocket->consumable->type)
        {
            case CONS_NOTHING: cout << "WARNING: Item.Use() not right. Item type: " << GetItemString(pocket->type) << "\n";
            case CONS_POTION: Heal(pocket->consumable->amount); break;
            case CONS_STATBOOST: cout << "Unimplemented STATBOOST item in Use()\n"; break;
            case CONS_BUFF: cout << "Unimplemented BUFF item in Use()\n"; break;
            default: cout << "UNIMPLEMENTED DEFAULT Item in USE()\n"; break;
        }
        cout << "Used item: " << GetItemString(pocket->type) << "\n";
        Discard();
    }

    // Deletes the unit's pocket item.
    void
    Discard()
    {
        delete pocket;
        pocket = nullptr;
    }

    bool
    PrimaryRange(int distance) const
    {
        if(weapon && weapon->weapon && 
           weapon->weapon->min_range <= distance &&
           weapon->weapon->max_range >= distance)
        {
            return true;
        }
        return false;
    }

    bool
    SecondaryRange(int distance) const
    {
        if(pocket && pocket->weapon && 
           pocket->weapon->min_range <= distance &&
           pocket->weapon->max_range >= distance)
        {
            return true;
        }
        return false;
    }

    int
    MinRange() const
    {
        if(weapon && weapon->weapon)
            return weapon->weapon->min_range;
        return 0;
    }
    int
    MaxRange() const
    {
        if(weapon && weapon->weapon)
            return weapon->weapon->max_range;
        return 0;
    }

    int
    OverallMinRange() const
    {
        int result = 0;
        if(weapon && weapon->weapon)
        {
            result = weapon->weapon->min_range;
        }
        if(pocket && pocket->weapon)
        {
            result = min(result, pocket->weapon->min_range);
        }
        return result;
    }

    int
    OverallMaxRange() const
    {
        int result = 0;
        if(weapon && weapon->weapon)
        {
            result = weapon->weapon->max_range;
        }
        if(pocket && pocket->weapon)
        {
            result = max(result, pocket->weapon->max_range);
        }
        return result;
    }

    // Damages a unit and resolves things involved with that process.
    void
    Damage(int amount)
    {
        health = clamp(health - amount, 0, MaxHealth());
    }

    // Damages a unit and resolves things involved with that process.
    void
    Heal(int amount)
    {
        health = clamp(health + amount, 0, MaxHealth());
    }

    void
    Deactivate()
    {
        is_exhausted = true;
        sheet.ChangeTrack(TRACK_IDLE);
    }
    void
    Activate()
    {
        is_exhausted = false;
    }
    void
    ApplyBuff(Buff *buff_in)
    {
        buff = buff_in;
    }

    // Called every turn. If buff is over, deletes the buff.
    void
    TickBuff()
    {
        --(buff->turns_remaining);
        if(buff->turns_remaining <= 0)
        {
            delete buff;
            buff = nullptr;
        }
    }

    bool
    GrantExperience(int amount)
    {
        if(level == 10)
            return false;;

        experience += amount;
        if(experience >= 100)
        {
            return true;
        }

        return false;
    }

    int MaxHealth() const
    {
        return max(1, 10 + (3 + (vitality / 2)) * level);
    }

    int Movement() const
    {
        return max(1, 4 + dexterity / 3);
    }

    int GetWeaponHitStat() const
    {
        if(!weapon)
            return 0;

        assert(weapon->weapon);
        switch(weapon->weapon->hit_stat)
        {
            case STAT_STRENGTH:  return strength;
            case STAT_DEXTERITY: return dexterity;
            case STAT_VITALITY:  return vitality;
            case STAT_INTUITION: return intuition;
            case STAT_FAITH:     return faith;
        }
    }

    int GetWeaponDmgStat() const
    {
        if(!weapon)
            return 0;

        assert(weapon->weapon);
        switch(weapon->weapon->hit_stat)
        {
            case STAT_STRENGTH:  return strength;
            case STAT_DEXTERITY: return dexterity;
            case STAT_VITALITY:  return vitality;
            case STAT_INTUITION: return intuition;
            case STAT_FAITH:     return faith;
        }
    }

    int
    ToHit() const
    {
        return (weapon ? GetWeaponHitStat() 
                       : max(strength, dexterity));
    }

    int
    DamageAmount() const
    {
        return (weapon ? weapon->weapon->RollDamage() + GetWeaponDmgStat()
                        : max(1, 1 + strength));
    }

    int
    AC() const
    {
        return 12;
    }

    int
    Crit() const
    {
        return 20;
    }
};

Unit *
GetUnitByName(const vector<shared_ptr<Unit>> &units, const string &name)
{
    for(shared_ptr<Unit> unit : units)
    {
        if(unit->ID() == hash<string>{}(name))
        {
            return unit.get();
        }
    }
    cout << "WARN GetUnitByName: No unit of that name: " << name << "\n";
    return nullptr;
}

// ===================================== Converation ===========================
enum Speaker
{
    SPEAKER_ONE,
    SPEAKER_TWO,
    SPEAKER_THREE,
    SPEAKER_FOUR,
};

enum ConversationEvent
{
    CONV_NONE,
    CONV_ONE_EXITS,
    CONV_TWO_EXITS,
    CONV_THREE_EXITS,
    CONV_FOUR_EXITS,
    CONV_ONE_ENTERS,
    CONV_TWO_ENTERS,
    CONV_THREE_ENTERS,
    CONV_FOUR_ENTERS,
};

Expression
GetExpressionFromString(const string &in)
{
    if(in == "Neutral") return EXPR_NEUTRAL;
    else if(in == "Happy") return EXPR_HAPPY;
    else if(in == "Angry") return EXPR_ANGRY;
    else if(in == "Wince") return EXPR_WINCE;
    cout << "Warning: Unsupported Expression in GetExpressionFromString: " << in << "\n";
    return EXPR_NEUTRAL;
}

ConversationEvent
GetConversationEventFromString(const string &in)
{
    if(in == "ONE Exits") return CONV_ONE_EXITS;
    else if(in == "TWO Exits") return CONV_TWO_EXITS;
    else if(in == "THREE Exits") return CONV_THREE_EXITS;
    else if(in == "FOUR Exits") return CONV_FOUR_EXITS;
    else if(in == "ONE Enters") return CONV_ONE_ENTERS;
    else if(in == "TWO Enters") return CONV_TWO_ENTERS;
    else if(in == "THREE Enters") return CONV_THREE_ENTERS;
    else if(in == "FOUR Enters") return CONV_FOUR_ENTERS;
    SDL_assert(!"Warning: Unsupported Expression in GetConversationEventFromString.");
    return CONV_ONE_EXITS;
}

struct Sentence
{
    Speaker speaker;
    string text;
    Expression expression;
    ConversationEvent event;
};

// CIRCULAR
Texture LoadTextureText(string, SDL_Color, int);

struct Conversation
{
    string filename = "";
    Unit *one = nullptr;
    Unit *two = nullptr;
    Unit *three = nullptr;
    Unit *four = nullptr;
    position pos = {-1, -1};
    vector<bool> active = {true, false, false, false};
    vector<Expression> expressions = {EXPR_NEUTRAL, EXPR_NEUTRAL, EXPR_NEUTRAL, EXPR_NEUTRAL};
    int current = 0;
    bool done = false;
    Sound *song = nullptr;
    vector<Sentence> prose;
    Texture words_texture;
    Texture speaker_texture;

    Conversation() = default;

    string
    Words() const
    {
        return prose[current].text;
    }

    Speaker
    Speaker() const
    {
        return prose[current].speaker;
    }

    Expression
    Expression() const
    {
        return prose[current].expression;
    }

    void
    ReloadTextures()
    {
        words_texture = LoadTextureText(Words(), black, CONVERSATION_WRAP);
        if(Speaker() == SPEAKER_ONE)
            speaker_texture = LoadTextureText(one->name, black, 0);
        else if(Speaker() == SPEAKER_TWO)
            speaker_texture = LoadTextureText(two->name, black, 0);
        else if(Speaker() == SPEAKER_TWO)
            speaker_texture = LoadTextureText(three->name, black, 0);
        else if(Speaker() == SPEAKER_TWO)
            speaker_texture = LoadTextureText(four->name, black, 0);
    }

    void
    Next()
    {
        ++current;
        if(current >= prose.size())
        {
            done = true;
            current = 0;
            return;
        }

        if(Speaker() == SPEAKER_ONE)
            expressions[0] = Expression();
        else if(Speaker() == SPEAKER_TWO)
            expressions[1] = Expression();
        else if(Speaker() == SPEAKER_THREE)
            expressions[2] = Expression();
        else if(Speaker() == SPEAKER_FOUR)
            expressions[3] = Expression();

        switch(prose[current].event)
        {
            case CONV_NONE:
            {
            } break;
            case CONV_ONE_EXITS: 
            {
                active[0] = false;
            } break;
            case CONV_TWO_EXITS: 
            {
                active[1] = false;
            } break;;
            case CONV_THREE_EXITS: 
            {
                active[2] = false;
            } break;;
            case CONV_FOUR_EXITS: 
            {
                active[3] = false;
            } break;;
            case CONV_ONE_ENTERS: 
            {
                active[0] = true;
            } break;;
            case CONV_TWO_ENTERS:
            {
                active[1] = true;
            } break;;
            case CONV_THREE_ENTERS:
            {
                active[2] = true;
            } break;;
            case CONV_FOUR_ENTERS:
            {
                active[3] = true;
            } break;;
            default: SDL_assert(!"ERROR Unhandled enum in Conversation.Next()");
        }
    }
};

typedef pair<int, Conversation> cutscene;

struct ConversationList
{
    vector<Conversation> list = {};
    int index = 0;
    vector<Conversation> mid_battle = {};
    vector<Conversation> villages = {};
    vector<cutscene> cutscenes = {};
    cutscene *current_cutscene = nullptr;
    Conversation *current_conversation = nullptr;
    Conversation prelude;
};


// ========================== map stuff =======================================
struct Tile
{
    TileType type = FLOOR;
    int penalty = 1;
    int avoid = 0;
    int defense = 0;
    Unit *occupant = nullptr;
    position atlas_index = {0, 16};
};

struct Tilemap
{
    int width;
    int height;
    vector<vector<Tile>> tiles = {};
    vector<position> accessible = {};
    vector<position> attackable = {};
    vector<position> ability = {};
    vector<position> range = {};
    vector<position> adjacent = {};
    vector<position> vis_range = {};

    // NOTE: For AI decision-making purposes
    vector<position> double_range = {};

    Texture atlas;
    int atlas_tile_size = ATLAS_TILE_SIZE;
};

enum Objective
{
    OBJECTIVE_ROUT,
    OBJECTIVE_CAPTURE,
    OBJECTIVE_BOSS,
};

// CIRCULAR: What a joke...
struct Level;
Level
LoadLevel(string filename_in, const vector<shared_ptr<Unit>> &units,
          const vector<shared_ptr<Unit>> &party);

struct Level
{
    string name = "";
    Objective objective;

    Tilemap map;
    vector<shared_ptr<Unit>> combatants;
    vector<shared_ptr<Unit>> bench;

    ConversationList conversations;
    Sound *song = nullptr;

    int turn_count = -1;
    //bool player_turn = false;
    bool next_level = false;
    bool turn_start = false;

    void
    CheckVictory()
    {
        if((objective == OBJECTIVE_ROUT &&
             GetNumberOf(false) == 0)
             ||
            (objective == OBJECTIVE_BOSS && 
             IsBossDead()))
        {
            song->FadeOut();
            GlobalInterfaceState = LEVEL_MENU;
            GlobalAIState = AI_NO_OP;
            EmitEvent(MISSION_COMPLETE_EVENT);
        }
    }

    Level
    LoadNextLevel(const string &name, 
                  const vector<shared_ptr<Unit>> &units,
                  vector<shared_ptr<Unit>> *party)
    {
        Level next;

        *party = {};
        for(shared_ptr<Unit> unit : combatants)
        {
        // Reset the party's statistics
            if(unit->is_ally)
            {
                unit->health = unit->MaxHealth();
                if(unit->buff)
                    delete unit->buff;
                    unit->buff = nullptr;
                unit->turns_active = -1;
                unit->is_exhausted = false;
                party->push_back(unit);
            }
        }

        song->Stop();
        next = LoadLevel(name, units, *party);
        next.conversations.prelude.song->Start();

        GlobalPlayerTurn = true;
        next.turn_start = true;

        return next;
    }

    bool
    CheckNextTurn()
    {
        if(!turn_start)
            return false;

        turn_start = false;

        if(GlobalPlayerTurn)
        {
            ++turn_count;

            for(auto const &unit : combatants)
            {
                if(!unit->is_ally) // Increment enemy units
                    ++unit->turns_active;
                if(unit->buff)
                    unit->TickBuff();
            }

            for(auto unit : bench)
            {
                if(unit->is_ally && (unit->arrival == turn_count))
                {
                    map.tiles[unit->pos.col][unit->pos.row].occupant = unit.get();
                    combatants.push_back(move(unit));
                }
            }
        }
        else
        {
            for(auto const &unit : combatants)
                if(unit->is_ally)
                    ++unit->turns_active;
        }

        for(auto const &unit : combatants)
            unit->Activate();

        return true;
    }

    // Puts a piece on the board
    void
    AddCombatant(shared_ptr<Unit> newcomer, const position &pos)
    {
        newcomer->pos = pos;
        combatants.push_back(newcomer);
        SDL_assert(!map.tiles[pos.col][pos.row].occupant);
        map.tiles[pos.col][pos.row].occupant = newcomer.get();
    }

    // Returns the position of the leader.
    position
    Leader()
    {
        for(const shared_ptr<Unit> &unit : combatants)
        {
            if(unit->ID() == LEADER_ID)
                return unit->pos;
        }
        SDL_assert(!"ERROR Level.Leader(): No leader!\n");
        return position(0, 0);
    }

    bool
    IsBossDead()
    {
        for(const shared_ptr<Unit> &unit : combatants)
        {
            if(unit->is_boss)
                return false;
        }
        return true;
    }

    void
    DetectDeadAllies()
    {
        for(shared_ptr<Unit> unit : combatants)
        {
            if(unit->should_die)
            {
                if(unit->is_ally && !unit->has_spoken_valediction)
                {
                    unit->should_die = false;
                    EmitEvent({UNIT_DEATH_EVENT, unit.get()});
                    GlobalInterfaceState = DEATH;
                    GlobalAIState = AI_DEATH;
                    return;
                }
            }
        }
    }

    void
    RemoveDeadUnits()
    {
        // Quit if Leader is dead
        if(GlobalInterfaceState != GAME_OVER)
        {
            position leader_pos = Leader();
            if(map.tiles[leader_pos.col][leader_pos.row].occupant->should_die)
            {
                GlobalInterfaceState = GAME_OVER;
                GlobalPlayerTurn = true;
                return;
            }
        }

        // REFACTOR: This could be so much simpler.
        vector<position> tiles;

        for(shared_ptr<Unit> unit : combatants)
            if(unit->should_die)
                tiles.push_back(unit->pos);

        combatants.erase(remove_if(combatants.begin(), combatants.end(),
                    [](auto const &u) { return u->should_die; }),
                    combatants.end());

        for(position tile : tiles)
            map.tiles[tile.col][tile.row].occupant = nullptr;
    }


    // A mutation function that just checks if there are any units left to
    // move, and ends the player's turn if there aren't.
    // TODO: This would be so much simpler as a function called when you deactivate a unit.
    // Look into this refactoring.
    void
    CheckForRemaining()
    {
        if(GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT)
        {
            for(auto const &u : combatants)
            {
                if(u->is_ally && !u->is_exhausted)
                    return;
            }

            // End player turn
            GlobalInterfaceState = NO_OP;
            GlobalAIState = AI_NO_OP;
            GlobalPlayerTurn = false;
            turn_start = true;
            EmitEvent(START_AI_TURN_EVENT);
        }
    }

    int
    GetNumberOf(bool is_ally = true) const
    {
        int result = 0;
        for(shared_ptr<Unit> unit : combatants)
        {
            if(unit->is_ally == is_ally)
                ++result;
        }
        return result;
    }

    void
    Update()
    {
        // cleanup functions
        DetectDeadAllies();
        RemoveDeadUnits();
        CheckForRemaining();
        CheckVictory();
    }
};

// ================================= Menu ======================================
struct Menu
{
    int rows = 0;
    int current = 0;

    vector<Texture> optionTextTextures;
    vector<string> optionText;

    Menu(vector<string> options_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), uiTextColor, 0));
            optionText.push_back(s);
            rows += 1;
        }
    }

    // Custom-build a menu based on your current options.
    void
    AddOption(string s)
    {
        rows += 1;
        optionTextTextures.push_back(LoadTextureText(s.c_str(), uiTextColor, 0));
        optionText.push_back(s);
    }
};

#endif
