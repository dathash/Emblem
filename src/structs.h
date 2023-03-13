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
    bool x;
    bool y;

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

    Texture(SDL_Texture *sdl_texture_in, 
            string dir_in, string filename_in, 
            int width_in, int height_in)
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

    Spritesheet(Texture texture_in, int size_in, int speed_in = ANIMATION_SPEED)
    : texture(texture_in),
      size(size_in),
      speed(speed_in)
    {
        this->tracks = texture_in.height / size_in;
        this->frames = texture_in.width / size_in;
    }

    // called each frame
    void Update() {
        counter++;
        if(!(counter % speed)) {
            int new_frame = frame + 1;
            if(new_frame >= frames)
                this->frame = 0;
            else
                this->frame = new_frame;
        }
    }

    // switches the sprite to the next animation track
    void ChangeTrack(SheetTrack track_in) {
        SDL_assert(track_in < tracks && track_in >= 0);
        this->track = track_in;
        this->frame = 0;
    }

    void ChangeTrackMovement(const direction &dir) {
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

// CIRCULAR
void GameOver();

struct Player
{
    int health = 7;
    int max_health = 7;
    int backup_health = 0;

    void Reset() {
        health = max_health;
    }

    void Damage(int amount) {
        health = clamp(health - amount, 0, max_health);
        if(health <= 0)
            GameOver(); // TODO: this results in a seg fault since we don't get rid of unresolved attacks.
    }

    void Heal(int amount) {
        health = clamp(health + amount, 0, max_health);
        // CONSIDER: grid defense
    }
};


enum Team
{
    TEAM_PLAYER,
    TEAM_AI,
    TEAM_ENV,
    TEAM_BUILDING,
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
    Equip *utility = nullptr;

    vector<Effect> effects = {};
    Effect passive = {EFFECT_NONE};

    position pos = {0, 0};
    position initial_pos = {0, 0};

    bool has_moved = false;
    bool is_exhausted = false;

    bool should_die = false;

    Spritesheet sheet;

    ~Unit() {
        delete primary;
        delete secondary;
        delete utility;
    }

    size_t ID() {
        return hash<string>{}(name);
    }

    bool IsAlly() const {
        return team == TEAM_PLAYER;
    }

    bool IsAI() const {
        return team == TEAM_AI;
    }

    bool IsEnv() const {
        return team == TEAM_ENV;
    }

    bool IsBuilding() const {
        return team == TEAM_BUILDING;
    }

    void Update() {
        sheet.Update();
    }

    Unit(
         string name_in, Team team_in,
         int health_in, int movement_in,
         bool fixed_in,
         Equip *primary_in,
         Equip *secondary_in,
         Equip *utility_in,
         EffectType effect_type_in,
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
      utility(utility_in),
      sheet(sheet_in)
    {
        passive = {effect_type_in};
    }

    Unit(const Unit &other)
    : name(other.name),
      team(other.team),
      health(other.health),
      max_health(other.max_health),
      movement(other.movement),
      fixed(other.fixed),
      passive(other.passive),
      sheet(other.sheet)
    {
        if(other.primary)
            primary = new Equip(*other.primary);
        if(other.secondary)
            secondary = new Equip(*other.secondary);
        if(other.utility)
            utility = new Equip(*other.utility);
    }

    // Damages a unit and resolves things involved with that process.
    // Returns the amount of damage actually done.
    int Damage(int amount) {
        if(GlobalGodMode && IsAlly()) return 0;
        if(GlobalGodMode && IsEnv()) return 0;

        int result = min(amount, health);
        health = clamp(health - amount, 0, max_health);
        if(health <= 0)
            should_die = true;
        return result;
    }

    // Heals a unit and resolves things involved with that process.
    void Heal(int amount) {
        health = clamp(health + amount, 0, max_health);
    }

    void ApplyEffect(EffectType type) {
        effects.push_back({type});
    }

    // Removes a given effect from the unit's list of temporary effects.
    // Does not touch their permanent passive.
    void RemoveEffect(EffectType type) {
        effects.erase(remove_if(effects.begin(), effects.end(),
                [type](const Effect &effect) { return effect.type == type; }),
                effects.end());
    }

    // Returns true if the unit has the given effect, false otherwise.
    bool HasEffect(EffectType type) {
        for(const Effect &effect : effects)
            if(effect.type == type)
                return true;
        return passive.type == type ? true : false;
    }

    void Deactivate() {
        is_exhausted = true;
        sheet.ChangeTrack(TRACK_IDLE);
    }

    void Activate() {
        has_moved = false;
        is_exhausted = false;
    }

};


shared_ptr<Unit>
GetUnitByName(const vector<shared_ptr<Unit>> &units, const string &name)
{
    for(shared_ptr<Unit> unit : units) {
        if(unit->ID() == hash<string>{}(name))
            return unit;
    }
    cout << "WARN GetUnitByName: No unit of that name: " << name << "\n";
    return nullptr;
}

// ========================== map stuff =======================================
enum TileType
{
    TILE_PLAIN,
    TILE_FORT,
    TILE_FLAME,
    TILE_WIND,
    TILE_STORM,
};
string 
GetTileTypeString(TileType type)
{
    switch (type)
    {
        case TILE_PLAIN:   return "Plains";
        case TILE_FORT:    return "Fort";
        case TILE_FLAME:   return "Flame";
        case TILE_WIND:    return "Wind";
        case TILE_STORM:   return "Storm";
	}
}


struct Tile
{
    position atlas_index = {0, 0};

    TileType type = TILE_PLAIN;
    EffectType effect = EFFECT_NONE;

    Unit *occupant = nullptr;
};
Tile
GetTile(TileType type)
{
    switch(type)
    {
        case(TILE_PLAIN):    return {{0, 0}, type, EFFECT_NONE};
        case(TILE_FORT):     return {{1, 0}, type, EFFECT_STONE};
        case(TILE_FLAME):    return {{2, 0}, type, EFFECT_AFLAME};
        case(TILE_WIND):     return {{3, 0}, type, EFFECT_SWIFT};
        case(TILE_STORM):    return {{4, 0}, type, EFFECT_PARALYZED};
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
        for(int row = (avoid_edges ? 1 : 0); row < (ai_side ? 5 : (avoid_edges ? MAP_HEIGHT - 1 : MAP_HEIGHT)); ++row) {
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


// CIRCULAR
void GoToAIPhase();

struct Level
{
    string name = "";

    Tilemap map;
    vector<shared_ptr<Unit>> combatants = {};
    Spawner spawner;

    Sound *song = nullptr;

    int turn_count = 0;
    int victory_turn = 1;

    bool can_undo = true;

    vector<shared_ptr<Unit>> to_warp = {};

    Level() = default;

    Level(const Level &other)
    : name(other.name),
      map(other.map),
      spawner(other.spawner),
      song(other.song),
      turn_count(other.turn_count),
      victory_turn(other.victory_turn),
      can_undo(other.can_undo)
    {
        for(shared_ptr<Unit> unit : other.combatants)
        {
            shared_ptr<Unit> copy = make_shared<Unit>(*unit);
            copy->pos = unit->pos;
            combatants.push_back(copy);
            map.tiles[copy->pos.col][copy->pos.row].occupant = copy.get();
        }
    }

    void TickEffect(shared_ptr<Unit> unit, const Effect &effect)
    {
        switch(effect.type)
        {
            case EFFECT_NONE:
            {
            } break;
            case EFFECT_AFLAME:
            {
                unit->Damage(1);
            } break;
            default:
            {
                cout << effect.type << ": This ability has no tick effect\n";
            } break;
        }
    }

    void TickEffects(Team team) {
        for(shared_ptr<Unit> unit : combatants) {
            if(unit->team == team) {
                for(const Effect &effect : unit->effects) {
                    TickEffect(unit, effect);
                }
                unit->effects = {};

                TickEffect(unit, unit->passive);
            }
        }

    }

    vector<shared_ptr<Unit>> Queue()
    {
        vector<shared_ptr<Unit>> result = {};
        for(auto unit : combatants)
        {
            if(unit->IsAI())
                result.push_back(unit);
        }
        return result;
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
    // For now, we just want 5 guys all the time.
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
                map.tiles[pos.col][pos.row].occupant->Damage(1); // For blocking a rising enemy
                keep.push_back(pos);
            }
        }

        spawner.rising = keep;
    }

    bool CheckVictory() {
        return turn_count >= victory_turn;
    }

    // Puts a piece on the board
    void AddCombatant(shared_ptr<Unit> newcomer, const position &pos) {
        newcomer->pos = pos;
        combatants.push_back(newcomer);
        SDL_assert(!map.tiles[pos.col][pos.row].occupant);
        map.tiles[pos.col][pos.row].occupant = newcomer.get();
    }

    // Returns the position of the leader.
    position
    FirstAlly()
    {
        for(const shared_ptr<Unit> &unit : combatants)
            if(unit->IsAlly()) return unit->pos;

        return position(-1, -1);
    }

    void
    RemoveDeadUnits()
    {
        // Quit if Leader is dead
        if(GlobalInterfaceState != GAME_OVER &&
           GlobalInterfaceState != WARP &&
           GlobalInterfaceState != TITLE_SCREEN)
        {
            position ally_pos = FirstAlly();
            if(ally_pos == position(-1, -1))
                GameOver();
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

    int GetNumberOf(Team team = TEAM_PLAYER) const {
        
        int result = 0;
        for(shared_ptr<Unit> unit : combatants)
        {
            if(unit->team == team)
                ++result;
        }
        return result;
    }
};

// ================================= Menu ======================================
// CIRCULAR
Texture LoadTextureText(string, SDL_Color, int, bool);

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
            optionTextTextures.push_back(LoadTextureText(s.c_str(), black, 0, false));
            optionText.push_back(s);
            rows += 1;
        }
    }

    // Custom-build a menu based on your current options.
    void
    AddOption(string s)
    {
        rows += 1;
        optionTextTextures.push_back(LoadTextureText(s.c_str(), black, 0, false));
        optionText.push_back(s);
    }
};

// =================================== Icons ===================================
// A struct which contains spritesheets used in visualizing attacks.
struct Icons
{
    Spritesheet arrow;

    Spritesheet dot;
    Spritesheet redarrow;

    Spritesheet rising;
    Spritesheet warning;

    Spritesheet aflame;
    Spritesheet para;
    Spritesheet swift;
    Spritesheet stone;

    Icons(Spritesheet arrow_in,
          Spritesheet dot_in,
          Spritesheet redarrow_in,
          Spritesheet rising_in,
          Spritesheet warning_in,
          Spritesheet aflame_in,
          Spritesheet para_in,
          Spritesheet swift_in,
          Spritesheet stone_in)
    : arrow(arrow_in),
      dot(dot_in),
      redarrow(redarrow_in),
      rising(rising_in),
      warning(warning_in),
      aflame(aflame_in),
      para(para_in),
      swift(swift_in),
      stone(stone_in)
    {}
};

#endif
