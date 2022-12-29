// Author: Alex Hartford
// Program: Emblem
// File: Tests
// Date: July 2022

#ifndef STRUCTS_H
#define STRUCTS_H


struct InputState
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;

	int joystickCooldown = 0;
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
        printf("WARN: Default texture constructor called.\n");
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
    void Update()
    {
        counter++;
        if(counter % speed == 0)
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
    void ChangeTrack(int track_in)
    {
        assert(track_in < tracks && track_in >= 0);
        this->track = track_in;
        this->frame = 0;
    }
};

// =================================== Gameplay ================================
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
    int healing = 0;
    int minRange;
    int maxRange;
    int accuracy;
    SpriteSheet sheet;
    Texture portrait;

    void Update()
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
         int minRange_in, int maxRange_in,
         int attack_in, int defense_in, int accuracy_in)
    : name(name_in),
      sheet(sheet_in),
      portrait(portrait_in),
      id(id_in),
      isAlly(isAlly_in),
      mov(mov_in),
      hp(hp_in),
      maxHp(maxHp_in),
      minRange(minRange_in),
      maxRange(maxRange_in),
      attack(attack_in),
      defense(defense_in),
      accuracy(accuracy_in)
    {} // haha c++
    // This little thing is like a vestigial organ
    // disgusting

    Unit(Unit &) = default;
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

//TODO: There's gotta be a better way than macros to set some default tile types
#define FLOOR_TILE {FLOOR, 1, 0, nullptr, {14, 1}}
#define WALL_TILE {WALL, 100, 0, nullptr, {6, 22}}
#define FOREST_TILE {FOREST, 2, 10, nullptr, {0, 6}}
#define DESERT_TILE {DESERT, 4, 0, nullptr, {18, 29}}
#define OBJECTIVE_TILE {OBJECTIVE, 1, 0, nullptr, {31, 0}}

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
    int width = 6;
    int height = 6;
    vector<vector<Tile>> tiles;
    vector<pair<int, int>> accessible;
    vector<pair<int, int>> interactible;
    Texture atlas;
    int atlas_tile_size = ATLAS_TILE_SIZE;
};

struct Level
{
    Tilemap map;
    vector<unique_ptr<Unit>> combatants;

    void RemoveDeadUnits()
    {
        vector<pair<int, int>> tiles;

        for(auto const &u : combatants)
        {
            if(u->shouldDie)
            {
                tiles.push_back(pair<int, int>(u->col, u->row)); 
            }
        }

        combatants.erase(remove_if(combatants.begin(), combatants.end(), [](auto const &u) { return u->shouldDie; }), combatants.end());

        for(pair<int, int> tile : tiles)
        {
            map.tiles[tile.first][tile.second].occupant = nullptr;
        }
    }
};

struct Cursor
{
    int col = 1;
    int row = 1;
    Unit *selected = nullptr;
    Unit *targeted = nullptr;
    int selectedCol = -1; // Where the cursor was before placing a unit
    int selectedRow = -1;
    int sourceCol = -1; // Where the cursor was before choosing a target
    int sourceRow = -1;

    int viewportSize = VIEWPORT_SIZE;
    int viewportCol = 0;
    int viewportRow = 0;

    SpriteSheet sheet;
    path path_draw = {};

    Cursor(SpriteSheet sheet_in)
    : sheet(sheet_in)
    {}

    void Update()
    {
        sheet.Update();
    }

    bool
    WithinViewport(int col, int row) const
    {
        return (col < viewportSize + viewportCol &&
                col >= viewportCol &&
                row < viewportSize + viewportRow &&
                row >= viewportRow);
    }

    // moves the cursor's viewport so that the given tile is on screen.
    void
    MoveViewport(int col, int row)
    {
        if(col >= viewportSize + viewportCol)
        {
            ++viewportCol;
        }
        else if(col < viewportCol)
        {
            --viewportCol;
        }
        else if(row >= viewportSize + viewportRow)
        {
            ++viewportRow;
        }
        else if(row < viewportRow)
        {
            --viewportRow;
        }
    }
};


// ============================= menu stuff ====================================
#include "load.h"
struct Menu
{
    u8 rows;
    u8 current;

    vector<Texture> optionTextTextures;

    Menu(u8 rows_in, u8 current_in, vector<string> options_in)
    : rows(rows_in),
      current(current_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};

struct TileInfo
{
    u8 rows;

    vector<Texture> infoTextTextures;

    int hp = 5;
    int maxHp = 10;

    TileInfo(u8 rows_in, vector<string> info_in)
    : rows(rows_in)
    {
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }

    void UpdateTextTextures(vector<string> info_in)
    {
        infoTextTextures.clear();
        int newRows = 0;
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
    }
};

struct UnitInfo
{
    u8 rows;
    vector<Texture> infoTextTextures;

    UnitInfo(u8 rows_in, vector<string> info_in)
    : rows(rows_in)
    {
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }

    void UpdateTextTextures(vector<string> info_in)
    {
        infoTextTextures.clear();
        int newRows = 0;
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
    }
};

// Finds the manhattan distance between two units.
int ManhattanDistance(const Unit &one, const Unit &two)
{
    return (abs(one.col - two.col) + abs(one.row - two.row));
}


struct CombatInfo
{
    u8 rows;

    vector<Texture> sourceTextTextures;
    vector<Texture> targetTextTextures;

    int unitHp = 5;
    int unitMaxHp = 10;
    int enemyHp = 5;
    int enemyMaxHp = 10;

    int unitDamage = 0;
    int enemyDamage = 0;

    bool attackerAttacking = false;
    bool victimAttacking = false;

    // TODO: Remove!!!
    // Determines what damage a hit will do.
    int CalculateDamage(int attack, int defense)
    {
        return max(attack - defense, 0);
    }

    CombatInfo(u8 rows_in, vector<string> sourceInfo_in, vector<string> targetInfo_in)
    : rows(rows_in)
    {
        for(string s : sourceInfo_in)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }

        for(string s : targetInfo_in)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        } 
    }

    void UpdatePreview(Unit &unit, Unit &enemy)
    {
        unitHp = unit.hp;
        unitMaxHp = unit.maxHp;
        enemyHp = enemy.hp;
        enemyMaxHp = enemy.maxHp;

        int dist = ManhattanDistance(unit, enemy);
        attackerAttacking = dist >= unit.minRange && dist <= unit.maxRange; 
        victimAttacking = dist >= enemy.minRange && dist <= enemy.maxRange; 
        printf("%d %d\n", attackerAttacking, victimAttacking);

        if(attackerAttacking)
        {
            unitDamage = CalculateDamage(unit.attack, enemy.defense);
        }
        else
        {
            unitDamage = 0;
        }
        if(victimAttacking)
        {
            enemyDamage = CalculateDamage(enemy.attack, unit.defense);
        }
        else
        {
            enemyDamage = 0;
        }

        vector<string> unitInfo =
        {
            unit.name,
            "N/A",
            "Hit: " + to_string(unit.accuracy),
            "Dmg: " + to_string(unitDamage),
        };

        vector<string> enemyInfo =
        {
            enemy.name,
            "N/A",
            "Hit: " + to_string(enemy.accuracy),
            "Dmg: " + to_string(enemyDamage),
        };

        sourceTextTextures.clear();
        targetTextTextures.clear();
        // Update textures with given strings
        int newRows = 0;
        for(string s : unitInfo)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
        for(string s : enemyInfo)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};

#endif
