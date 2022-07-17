// Author: Alex Hartford
// Program: Emblem
// File: Tests
// Date: July 2022

#ifndef STRUCTS_H
#define STRUCTS_H

// ============================ structs ====================================
struct InputState
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
};


struct Texture
{
    SDL_Texture *sdlTexture;
    string path;
    int width;
    int height;

    Texture(SDL_Texture *sdlTexture_in, string path_in, int width_in, int height_in)
    {
        this->sdlTexture = sdlTexture_in;
        this->width = width_in;
        this->height = height_in;
        this->path = path_in;
    }
};

struct SpriteSheet
{
    Texture texture;
    int size    = 32;
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
        if(track_in >= tracks || track_in < 0)
        {
            assert(!"SpriteSheet | ChangeTrack | Invalid Animation Track!\n");
        }
        this->track = track_in;
        this->frame = 0;
    }
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
    int healing = 0;
    int minRange;
    int maxRange;
    int accuracy;
    SpriteSheet sheet;

    void Update()
    {
        sheet.Update();
    }

    Unit(string name_in, SpriteSheet sheet_in,
         int id_in, bool isAlly_in, int mov_in,
         int hp_in, int maxHp_in,
         int minRange_in, int maxRange_in,
         int attack_in, int defense_in, int accuracy_in)
    : name(name_in),
      sheet(sheet_in),
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
    {}
};


// map stuff
enum TileTypes
{
    FLOOR,
    WALL,
    OBJECTIVE,
};
struct Tile
{
    int type = 0;
    int penalty = 1;
    int avoid = 0;
    bool occupied = false;
    Unit *occupant = nullptr;
};
struct Tilemap
{
    int width = 6;
    int height = 6;
    vector<vector<Tile>> tiles;
    vector<pair<int, int>> accessible;
    vector<pair<int, int>> interactible;
};

struct Level
{
    Tilemap map;
    vector<unique_ptr<Unit>> allies;
    vector<unique_ptr<Unit>> enemies;

    void RemoveDeadUnits()
    {
        vector<pair<int, int>> tiles;

        for(auto const &u : allies)
        {
            if(u->shouldDie)
            {
                tiles.push_back(pair<int, int>(u->col, u->row)); 
            }
        }
        for(auto const &u : enemies)
        {
            if(u->shouldDie)
            {
                tiles.push_back(pair<int, int>(u->col, u->row));
            }
        }

        allies.erase(remove_if(allies.begin(), allies.end(), [](auto const &u) { return u->shouldDie; }), allies.end());
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](auto const &u) { return u->shouldDie; }), enemies.end());

        for(pair<int, int> tile : tiles)
        {
            map.tiles[tile.first][tile.second].occupant = nullptr;
            map.tiles[tile.first][tile.second].occupied = false;
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

    int viewportSize = 8;
    int viewportCol = 0;
    int viewportRow = 0;

    SpriteSheet sheet;

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


// menu stuff
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

        unitDamage = CalculateDamage(unit.attack, enemy.defense);
        enemyDamage = CalculateDamage(enemy.attack, unit.defense);

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
