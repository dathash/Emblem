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
struct Growths
{
    int health;
    int attack;
    int aptitude;
    int defense;
    int speed;
    int skill;
};

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


struct Boosts
{
    int health = 0;
    int attack = 0;
    int aptitude = 0;
    int defense = 0;
    int speed = 0;
    int skill = 0;
};


struct Unit
{
    string name;
    bool is_ally;
    int movement;
    int health;
    int max_health;
    int attack;
    int aptitude;
    int defense;
    int speed;
    int skill;
    int min_range;
    int max_range;
    int level;
    Ability ability;

    Growths growths = {};
    int experience = 0;

    int arrival = 0;
    int turns_active = -1;
    int xp_value = 0;
    AIBehavior ai_behavior = NO_BEHAVIOR;
    position pos = {0, 0};
    bool is_exhausted = false;
    bool should_die = false;
    position animation_offset = {0, 0};
    bool is_boss = false;

    Buff *buff = nullptr;

    Spritesheet sheet;
    Texture neutral;
    Texture happy;
    Texture angry;
    Texture wince;

    ~Unit()
    {
        delete buff;
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
         string name_in,
         bool is_ally_in, int movement_in,
         int health_in, int max_health_in,
         int attack_in, int aptitude_in, int defense_in,
         int speed_in, int skill_in,
         int min_range_in, int max_range_in,
         int level_in,
         Ability ability_in,
         AIBehavior ai_behavior_in,
         int xp_value_in,

         int health_growth_in,
         int attack_growth_in,
         int aptitude_growth_in,
         int defense_growth_in,
         int speed_growth_in,
         int skill_growth_in,

         Spritesheet sheet_in,
         Texture neutral_in,
         Texture happy_in,
         Texture angry_in,
         Texture wince_in
         )
    : name(name_in),
      is_ally(is_ally_in),
      movement(movement_in),
      health(health_in),
      max_health(max_health_in),
      attack(attack_in),
      aptitude(aptitude_in),
      defense(defense_in),
      speed(speed_in),
      skill(skill_in),
      min_range(min_range_in),
      max_range(max_range_in),
      level(level_in),
      ability(ability_in),
      ai_behavior(ai_behavior_in),
      xp_value(xp_value_in),
      sheet(sheet_in),
      neutral(neutral_in),
      happy(happy_in),
      angry(angry_in),
      wince(wince_in)
    {
      growths.health = health_growth_in;
      growths.attack = attack_growth_in;
      growths.aptitude = aptitude_growth_in;
      growths.defense = defense_growth_in;
      growths.speed = speed_growth_in;
      growths.skill = skill_growth_in;
    }

    // Damages a unit and resolves things involved with that process.
    void
    Damage(int amount)
    {
        health = clamp(health - amount, 0, max_health);
    }

    // Damages a unit and resolves things involved with that process.
    void
    Heal(int amount)
    {
        health = clamp(health + amount, 0, max_health);
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

    int
    StatBoost(int growth)
    {
        int result = 0;
        while(growth > 100)
        {
            result += 1;
            growth -= 100;
        }

        if(d100() < growth)
        {
            result += 1;
        }

        return result;
    }

    Boosts
    CalculateLevelUp()
    {
        Boosts boosts;

        boosts.health = StatBoost(growths.health);
        boosts.attack = StatBoost(growths.attack);
        boosts.defense = StatBoost(growths.defense);
        boosts.aptitude = StatBoost(growths.aptitude);
        boosts.speed = StatBoost(growths.speed);
        boosts.skill = StatBoost(growths.skill);

        return boosts;
    }

    void
    LevelUp(Boosts boosts)
    {
        level += 1;
        
        health += boosts.health;
        attack += boosts.attack;
        defense += boosts.defense;
        aptitude += boosts.aptitude;
        speed += boosts.speed;
        skill += boosts.skill;

        experience -= 100;
        if(level == 10)
            experience = 0;
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

    int
    Accuracy() const
    {
        return 50 + (skill * 10);
    }

    int
    Avoid() const
    {
        return skill * 5;
    }

    int
    Crit() const
    {
        return skill * 2;
        // TODO: Make this less for enemies.
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
    cout << "WARN GetUnitByName: No unit of that name.\n";
    return nullptr;
}

// ===================================== Converation ===========================
enum Speaker
{
    SPEAKER_ONE,
    SPEAKER_TWO,
};

enum ConversationEvent
{
    CONV_NONE,
    CONV_ONE_EXITS,
    CONV_TWO_EXITS,
    CONV_ONE_ENTERS,
    CONV_TWO_ENTERS,
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
    else if(in == "ONE Enters") return CONV_ONE_ENTERS;
    else if(in == "TWO Enters") return CONV_TWO_ENTERS;
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
    position pos = {-1, -1};
    pair<bool, bool> active = {true, false};
    pair<Expression, Expression> expressions = {EXPR_NEUTRAL, EXPR_NEUTRAL};
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
        if(Speaker() == SPEAKER_TWO)
            speaker_texture = LoadTextureText(two->name, black, 0);
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
        {
            expressions.first = Expression();
        }
        else if(Speaker() == SPEAKER_TWO)
        {
            expressions.second = Expression();
        }

        switch(prose[current].event)
        {
            case CONV_NONE:
            {
            } break;
            case CONV_ONE_EXITS: 
            {
                active.first = false;
            } break;
            case CONV_TWO_EXITS: 
            {
                active.second = false;
            } break;;
            case CONV_ONE_ENTERS: 
            {
                active.first = true;
            } break;;
            case CONV_TWO_ENTERS:
            {
                active.second = true;
            } break;;
            default: SDL_assert(!"ERROR Unhandled enum in Conversation.Next()");
        }
    }
};

struct ConversationList
{
    vector<Conversation> list = {};
    int index = 0;
    vector<Conversation> mid_battle = {};
    vector<Conversation> villages = {};
    Conversation *current = nullptr;
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
                unit->health = unit->max_health;
                if(unit->buff)
                    delete unit->buff;
                    unit->buff = nullptr;
                unit->turns_active = -1;
                unit->is_exhausted = false;
                party->push_back(unit);
            }
        }

        song->Stop();
        next = LoadLevel(DATA_PATH + name, units, *party);
        next.conversations.prelude.song->Start();

        GlobalPlayerTurn = true;
        next.turn_start = true;

        return next;
    }

    bool
    CheckNextTurn()
    {
        if(turn_start)
        {
            turn_start = false;

            if(GlobalPlayerTurn)
            {
                ++turn_count;
                for(auto const &unit : combatants)
                {
                    if(!unit->is_ally) // Increment enemy units
                    {
                        ++unit->turns_active;
                    }
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
                {
                    if(unit->is_ally)
                    {
                        ++unit->turns_active;
                    }
                }
            }

            for(auto const &unit : combatants)
                unit->Activate();

            return true;
        }

        return false;
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
    RemoveDeadUnits()
    {
        if(GlobalInterfaceState != GAME_OVER)
        {
            position leader_pos = Leader();
            // Quit if Leader is dead
            if(map.tiles[leader_pos.col][leader_pos.row].occupant->should_die)
            {
                GlobalInterfaceState = GAME_OVER;
                GlobalPlayerTurn = true;
                return;
            }
        }

        // REFACTOR: This could be so much simpler.
        vector<position> tiles;
        for(const shared_ptr<Unit> &unit : combatants)
        {
            if(unit->should_die)
            {
                tiles.push_back(unit->pos);
            }
        }

        combatants.erase(remove_if(combatants.begin(), combatants.end(),
                    [](auto const &u) { return u->should_die; }),
                    combatants.end());

        for(position tile : tiles)
            map.tiles[tile.col][tile.row].occupant = nullptr;
    }

    // A mutation function that just checks if there are any units left to
    // move, and ends the player's turn if there aren't.
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
            GlobalPlayerTurn = false;
            turn_start = true;
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
        RemoveDeadUnits();
        CheckForRemaining();
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
