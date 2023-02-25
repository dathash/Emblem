// Author: Alex Hartford
// Program: Emblem
// File: Structs

#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>

void GoToAIPhase();
struct Level;
struct Cursor;
void GoToPlayerPhase(Level *level, Cursor *cursor);
void GoToResolutionPhase();
void GameOver();

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

    Texture() = default;
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
struct Player
{
    int health = 7;
    int max_health = 7;

    void
    Reset()
    {
        health = max_health;
    }

    void
    Damage(int amount)
    {
        health = clamp(health - amount, 0, max_health);
        if(health == 0)
            GameOver(); // TODO: this results in a seg fault since we don't get rid of unresolved attacks.
    }
};

enum Team
{
    TEAM_PLAYER,
    TEAM_AI,
    TEAM_ENV,
};

struct Unit
{
    string name;
    Team team;
    int health;
    int max_health;
    int movement;
    bool fixed;

    Equip *primary = nullptr;
    Equip *secondary = nullptr;

    position pos = {0, 0};
    position initial_pos = {0, 0};

    bool has_moved = false;
    bool is_exhausted = false;

    bool should_die = false;

    Spritesheet sheet;


    ~Unit()
    {
        delete primary;
        delete secondary;
    }

    size_t
    ID()
    {
        return hash<string>{}(name);
    }

    bool
    IsAlly() const
    {
        return team == TEAM_PLAYER;
    }

    bool
    IsAI() const
    {
        return team == TEAM_AI;
    }

    bool
    IsEnv() const
    {
        return team == TEAM_ENV;
    }

    void
    Update()
    {
        sheet.Update();
    }

    Unit(
         string name_in, Team team_in,
         int health_in, int movement_in,
         bool fixed_in,
         Equip *primary_in,
         Equip *secondary_in,
         Spritesheet sheet_in
         )
    : name(name_in),
      team(team_in),
      health(health_in),
      max_health(health_in),
      movement(movement_in),
      fixed(fixed_in),
      primary(primary_in),
      secondary(secondary_in),
      sheet(sheet_in)
    {}

    Unit(const Unit &other)
    : name(other.name),
      team(other.team),
      health(other.health),
      max_health(other.max_health),
      movement(other.movement),
      fixed(other.fixed),
      sheet(other.sheet)
    {
        if(other.primary)
            primary = new Equip(*other.primary);
        if(other.secondary)
            secondary = new Equip(*other.secondary);
    }

    // Damages a unit and resolves things involved with that process.
    // Returns the amount of damage actually done.
    int
    Damage(int amount)
    {
        int result = min(amount, health);
        health = clamp(health - amount, 0, max_health);
        return result;
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
        has_moved = false;
        is_exhausted = false;
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
// ========================== map stuff =======================================
enum TileType
{
    TILE_FLOOR,
    TILE_WATER,
    TILE_FLAME,
    TILE_ACID,
    TILE_SMOKE,
    TILE_DESERT,
    TILE_WALL,
    TILE_FOREST,
    TILE_SWAMP,
    TILE_GOAL,
    TILE_FORT,
    TILE_VILLAGE,
    TILE_CHEST,
};
string 
GetTileTypeString(TileType type)
{
    switch (type)
    {
    case TILE_FLOOR:   return "Plain";
    case TILE_WATER:   return "Water";
    case TILE_FLAME:   return "Flame";
    case TILE_ACID:    return "Acid";
    case TILE_SMOKE:   return "Smoke";
    case TILE_DESERT:  return "Desert";
    case TILE_WALL:    return "Hill";
    case TILE_FOREST:  return "Forest";
    case TILE_SWAMP:   return "Swamp";
    case TILE_GOAL:    return "Goal";
    case TILE_FORT:    return "Fort";
    case TILE_VILLAGE: return "House";
    case TILE_CHEST:   return "Chest";
	}
}

enum TileEffect
{
    EFFECT_NONE,
    EFFECT_FIRE,
    EFFECT_ACID,
    EFFECT_SMOKE,
    EFFECT_WATER,
};
string 
GetTileEffectString(TileEffect type)
{
    switch(type)
    {
    case EFFECT_NONE:  return "No tile effect";
    case EFFECT_FIRE:  return "Fire";
    case EFFECT_ACID:  return "Acid";
    case EFFECT_SMOKE: return "Smoke";
    case EFFECT_WATER: return "Water";
	}
}


struct Tile
{
    position atlas_index = {0, 0};

    TileType type = TILE_FLOOR;

    TileEffect effect = EFFECT_NONE;
    Unit *occupant = nullptr;
};
Tile
GetTile(TileType type)
{
    switch(type)
    {
        case(TILE_FLOOR):    return {{0, 0}, type};
        case(TILE_WATER):    return {{3, 0}, type};
        case(TILE_FLAME):    return {{0, 2}, type};
        case(TILE_ACID):     return {{1, 2}, type};
        case(TILE_SMOKE):    return {{3, 2}, type};
        case(TILE_DESERT):   return {{2, 2}, type};
        case(TILE_WALL):     return {{1, 0}, type};
        case(TILE_FOREST):   return {{2, 0}, type};
        case(TILE_SWAMP):    return {{5, 1}, type};
        case(TILE_GOAL):     return {{5, 0}, type};
        case(TILE_FORT):     return {{4, 0}, type};
        case(TILE_VILLAGE):  return {{1, 1}, type};
        case(TILE_CHEST):    return {{0, 1}, type};
    }
}

struct Tilemap
{
    Tile tiles[MAP_WIDTH][MAP_HEIGHT] = {};
    vector<position> accessible = {};
    vector<position> range = {};
    vector<position> attackable = {};

    Texture atlas;
    int atlas_tile_size = ATLAS_TILE_SIZE;
};

// Returns a copy of a random unit in a pool.
shared_ptr<Unit>
GetRandomUnit(const vector<shared_ptr<Unit>> &pool) {
    return make_shared<Unit>(*pool[RandomInt(pool.size() - 1)]);
}


// Finds an available space on the given map, while avoiding tiles in the mask.
// the ai_side parameter ensures the space is on the
position
GetOpenSpace(const Tilemap &map, const vector<position> &mask, bool ai_side, bool avoid_edges = false) {
    vector<position> choices = {};
    for(int col = (avoid_edges ? 1 : 0); col < (avoid_edges ? MAP_WIDTH - 1 : MAP_WIDTH); ++col) {
        for(int row = (ai_side ? 4 : (avoid_edges ? 1 : 0)); row < (avoid_edges ? MAP_HEIGHT - 1 : MAP_HEIGHT); ++row) {
            if(!map.tiles[col][row].occupant && !VectorContains({col, row}, mask))
                choices.push_back({col, row});
        }
    }
    if(choices.empty())
    {
        cout << "WARN GetOpenSpace: No open spaces!\n";
        return {-1, -1};
    }

    return choices[RandomInt(choices.size() - 1)];
}

struct Spawner
{
    vector<shared_ptr<Unit>> pool = {};
    vector<position> rising = {};
};


struct Level
{
    string name = "";

    Tilemap map;
    vector<shared_ptr<Unit>> combatants;
    Spawner spawner;

    Sound *song = nullptr;

    int turn_count = -1;

    void SpawnPhase() {
        SpawnUnits();
        SetRisingPoints(map);
    }

    // TODO:
    // Starting point for balancing spawns:
    // We want a total number of threats based on a per-turn basis.
    // This means combined surface and rising monsters.

    //         E | N | H
    // Turn 1: 4 | 5 | 6
    // Turn 2: 4 | 5 | 6
    // Turn 3: 5 | 6 | 7
    // Turn 4: 5 | 6 | 7
    void
    SetRisingPoints(const Tilemap &map)
    {
        int number_on_field = GetNumberOf(TEAM_AI);
        int number_rising = spawner.rising.size();
        int total_enemies = number_on_field + number_rising;

        int spawn_amount = 5 - total_enemies;

        for(int i = 0; i < spawn_amount; ++i)
            spawner.rising.push_back(GetOpenSpace(map, spawner.rising, true));
    }

    void
    SpawnUnits()
    {
        if(spawner.pool.empty())
        {
            cout << "WARN: Spawner pool is empty. Spawning nothing.\n";
            return;
        }

        vector<position> keep = {};

        for(const position &pos : spawner.rising)
        {
            if(!map.tiles[pos.col][pos.row].occupant)
            {
                shared_ptr<Unit> copy = GetRandomUnit(spawner.pool);
                AddCombatant(copy, pos);
            }
            else
            {
                keep.push_back(pos);
            }
        }

        spawner.rising = keep;
    }

    void
    CheckVictory()
    {
        if(false)
        {
            song->FadeOut();
            EmitEvent(MISSION_COMPLETE_EVENT);
        }
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

    void
    RemoveDeadUnits()
    {
        // Quit if Leader is dead
        if(GlobalInterfaceState != GAME_OVER)
        {
            position leader_pos = Leader();
            if(map.tiles[leader_pos.col][leader_pos.row].occupant->should_die)
            {
                GameOver();
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
                if(u->IsAlly() && !u->is_exhausted)
                    return;
            }

            GoToAIPhase();
        }
    }

    int GetNumberOf(Team team = TEAM_PLAYER) const {
        
        int result = 0;
        for(shared_ptr<Unit> unit : combatants)
        {
            if(unit->team == team)
                ++result;
        }
        return result;
    }

    void
    Update()
    {
        //CheckForRemaining();
        //CheckVictory();
    }
};

// ================================= Menu ======================================
Texture
LoadTextureText(string text, SDL_Color color, int line_length);
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
