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

    //Equip *primary = nullptr;
    //Equip *secondary = nullptr;

    position pos = {0, 0};
    position initial_pos = {0, 0};

    bool has_moved = false;
    bool is_exhausted = false;

    bool should_die = false;

    Spritesheet sheet;

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

    void
    Update()
    {
        sheet.Update();
    }

    Unit(
         string name_in, Team team_in,
         int health_in, int movement_in,
         Spritesheet sheet_in
         )
    : name(name_in),
      team(team_in),
      max_health(health_in),
      health(health_in),
      movement(movement_in),
      sheet(sheet_in)
    {}

    Unit(const Unit &other)
    : name(other.name),
      team(other.team),
      max_health(other.max_health),
      health(other.health),
      movement(other.movement),
      sheet(other.sheet)
    {}

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
struct Tile
{
    TileType type = FLOOR;
    Unit *occupant = nullptr;
    position atlas_index = {0, 16};
};

struct Tilemap
{
    Tile tiles[MAP_WIDTH][MAP_HEIGHT] = {};
    vector<position> accessible = {};
    vector<position> range = {};
    vector<position> attackable = {};

    Texture atlas;
    int atlas_tile_size = ATLAS_TILE_SIZE;
};

struct Level
{
    string name = "";

    Tilemap map;
    vector<shared_ptr<Unit>> combatants;

    Sound *song = nullptr;

    int turn_count = -1;

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
        if(GlobalInterfaceState == NEUTRAL_DEACTIVATED_UNIT)
        {
            for(auto const &u : combatants)
            {
                if(u->IsAlly() && !u->is_exhausted)
                    return;
            }

            GoToAIPhase();
        }
    }

    int
    GetNumberOf(Team team = TEAM_PLAYER) const
    {
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
        CheckForRemaining();
        CheckVictory();
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
