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
    bool r;

	int joystickCooldown = 0;
};

struct Timer
{
    Uint32 current = 0;
    Uint32 last_frame = 0;
    Uint32 end;
    bool paused = false;

    Timer(int seconds)
    {
        last_frame = SDL_GetTicks();
        end = seconds * 1000;
    }

    Timer()
    {} // NOTE: This is c++ weirdness. I'm sure I could figure it out if I
       // gave it a few minutes.

    bool
    Update()
    {
        if(paused)
            return false;

        int since_last_frame = SDL_GetTicks() - last_frame;

        if(since_last_frame > 30) // Quick hack to fix pausing
            since_last_frame = 30; // TODO: Really fix this someday

        current += since_last_frame;

        last_frame = SDL_GetTicks();
        if(current >= end)
            return true;
        return false;
    }

    void
    Pause()
    {
        paused = true;
    }

    void
    Start()
    {
        paused = false;
        last_frame = SDL_GetTicks();
    }
};

// ================================= Rendering =================================
struct Texture
{
    SDL_Texture *sdlTexture;
    string filename;
    string dir;
    int width;
    int height;

    Texture(SDL_Texture *sdlTexture_in, string dir_in, string filename_in, int width_in, int height_in)
    {
        this->sdlTexture = sdlTexture_in;
        this->width = width_in;
        this->height = height_in;
        this->dir = dir_in;
        this->filename = filename_in;
    }

    Texture()
    {
        //printf("WARN: Default texture constructor called.\n");
    }
};

struct SpriteSheet
{
    Texture texture;
    int size    = SPRITE_SIZE;
    int tracks  = 0;
    int frames  = 0;
    int track   = 0;
    int frame   = 0;
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;

    SpriteSheet(Texture texture_in, int size_in, int speed_in)
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
            int newFrame = frame + 1;
            if(newFrame >= frames)
            {
                this->frame = 0;
            }
            else
            {
                this->frame = newFrame;
            }
        }
    }

    // switches the sprite to the next animation track
    void
    ChangeTrack(int track_in)
    {
        assert(track_in < tracks && track_in >= 0);
        this->track = track_in;
        this->frame = 0;
    }
};

// =================================== Gameplay ================================
enum AIBehavior
{
    NO_BEHAVIOR,
    PURSUE,
    BOLSTER,
    FLEE
};

struct Unit
{
    string name;
    int id;
    bool isAlly;
    int col = 0;
    int row = 0;
    bool isExhausted = false;
    bool shouldDie = false;
    int mov;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int healing;
    int minRange;
    int maxRange;
    int accuracy;
    int avoid;
    int crit;
    SpriteSheet sheet;
    Texture portrait;
    AIBehavior ai_behavior = NO_BEHAVIOR;

    void
    Update()
    {
        sheet.Update();
		if(hp <= 0)
		{
			shouldDie = true;
		}
    }

    Unit(string name_in, SpriteSheet sheet_in,
         Texture portrait_in,
         int id_in, bool isAlly_in, int mov_in,
         int hp_in, int maxHp_in,
         int attack_in, int defense_in,
         int accuracy_in, int avoid_in, int crit_in,
         int minRange_in, int maxRange_in,
         AIBehavior ai_behavior_in)
    : name(name_in),
      sheet(sheet_in),
      portrait(portrait_in),
      id(id_in),
      isAlly(isAlly_in),
      mov(mov_in),
      hp(hp_in),
      maxHp(maxHp_in),
      attack(attack_in),
      defense(defense_in),
      accuracy(accuracy_in),
      avoid(avoid_in),
      crit(crit_in),
      minRange(minRange_in),
      maxRange(maxRange_in),
      ai_behavior(ai_behavior_in)
    {} // haha c++
    // This little thing is like a vestigial organ
    // disgusting


    // Damages a unit and resolves things involved with that process.
    // Includes a clamp function for less code reuse
    void
    Damage(int dmg)
    {
        hp = clamp(hp - dmg, 0, maxHp);
    }

    void
    Deactivate()
    {
        isExhausted = true;
        sheet.ChangeTrack(0);
    }

    void
    Activate()
    {
        isExhausted = false;
    }
};


// ========================== map stuff =======================================
enum TileTypes
{
    FLOOR,
    WALL,
    FOREST,
    DESERT,
    OBJECTIVE,
};

struct Tile
{
    int type = 0;
    int penalty = 1;
    int avoid = 0;
    Unit *occupant = nullptr;
    point atlas_index = {0, 16};
};
struct Tilemap
{
    int width;
    int height;
    vector<vector<Tile>> tiles;
    vector<point> accessible;
    vector<point> attackable;
    vector<point> healable;
    //CONSIDER: For trading, talking, playing rock-paper scissors, haha whatever
    //vector<point> adjacent;
    Texture atlas;
    int atlas_tile_size = ATLAS_TILE_SIZE;

    // This is for if we want to allow editors to change size of levels midway
    // through editing. Not super important, but it's here.
    //void
    //Resize()
    //{
    //    level.map.tiles.resize(level.map.width, vector<Tile>(level.map.height));
    //    for(int col = 0; col < level.map.width; ++col)
    //    {
    //        for(int row = 0; row < level.map.height; ++row)
    //        {
    //            level.map.tiles[col][row] = {};
    //        }
    //    }
    //}
};

struct Level
{
    Tilemap map;
    vector<unique_ptr<Unit>> combatants;
    Timer timer;

    // Returns the position of the leader.
    point
    Leader()
    {
        for(const unique_ptr<Unit> &unit : combatants)
        {
            if(unit->id == LEADER_ID)
                return point(unit->col, unit->row);
        }
        assert(!"ERROR Level.Leader(): No leader!\n");
    }

    void
    RemoveDeadUnits()
    {
        point leaderPosition = Leader();
        // Quit if Zarathustra's dead
        if(map.tiles[leaderPosition.first][leaderPosition.second].occupant->shouldDie)
        {
            printf("Zarathustra died. Game over!\n");
            RestartLevel();
            return;
        }

        // REFACTOR: This could be so much simpler.
        vector<point> tiles;
        for(const unique_ptr<Unit> &unit : combatants)
        {
            if(unit->shouldDie)
                tiles.push_back(point(unit->col, unit->row));
        }

        combatants.erase(remove_if(combatants.begin(), combatants.end(),
                    [](auto const &u) { return u->shouldDie; }),
                    combatants.end());

        for(point tile : tiles)
        {
            map.tiles[tile.first][tile.second].occupant = nullptr;
        }
    }

    // A mutation function that just checks if there are any units left to
    // move, and ends the player's turn if there aren't.
    void
    CheckForRemaining()
    {
        for(auto const &u : combatants)
        {
            if(u->isAlly && !u->isExhausted)
                return;
        }
        EndPlayerTurn();
        return;
    }

    void
    Update()
    {
        // cleanup functions
        RemoveDeadUnits();
        CheckForRemaining();
        if(timer.Update())
        {
            printf("Ran out of time. Game over!\n");
            RestartLevel();
        }
    }
};

struct Cursor
{
    int col = -1;
    int row = -1;
    Unit *selected = nullptr;
    Unit *targeted = nullptr;
    int selectedCol = -1; // Where the cursor was before placing a unit
    int selectedRow = -1;
    int sourceCol = -1; // Where the cursor was before choosing a target
    int sourceRow = -1;

    SpriteSheet sheet;
    path path_draw = {};

    Cursor(SpriteSheet sheet_in)
    : sheet(sheet_in)
    {}

    void
    Update()
    {
        sheet.Update();
    }

    // Places the cursor at a position.
    void
    PlaceAt(const point &p)
    {
        col = p.first;
        row = p.second;
        path_draw = {};
        //TODO: What if the leader starts outside of the 0, 0 viewport?
    }

    // returns the current quadrant of where the cursor is on the screen.
    enum quadrant
    Quadrant() const
    {
        int x = col - viewportCol;
        int y = row - viewportRow;

        if(x > VIEWPORT_WIDTH / 2 && y >= VIEWPORT_HEIGHT / 2)
        {
            return BOTTOM_RIGHT;
        }
        else if(x > VIEWPORT_WIDTH / 2)
        {
            return TOP_RIGHT;
        }
        else if(y >= VIEWPORT_HEIGHT / 2)
        {
            return BOTTOM_LEFT;
        }
        return TOP_LEFT;
    }
};

#endif
