// Author: Alex Hartford
// Program: Emblem
// File: Load

#ifndef LOAD_H
#define LOAD_H

#include <fstream>
#include <sstream>
#include <vector>

Tile
TileTypeToTile(TileType type)
{
    switch(type)
    {
        case(FLOOR):
            return FLOOR_TILE;
        case(WALL):
            return WALL_TILE;
        case(FOREST):
            return FOREST_TILE;
        case(SWAMP):
            return SWAMP_TILE;
        case(FORT):
            return FORT_TILE;
        case(GOAL):
            return GOAL_TILE;
        case(VILLAGE):
            return VILLAGE_TILE;
        case(CHEST):
            return CHEST_TILE;
        default: cout << "ERROR: Unhandled tile type in load.h::TileTypeToTile!\n"; return {};
    }
}

// ============================== loading data =================================
// Loads a Texture displaying the given text in the given color.
// Now with wrapping, set by the line_length parameter
Texture
LoadTextureText(string text, SDL_Color color, int line_length)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    SDL_assert(GlobalFont);
    surface = TTF_RenderText_Solid_Wrapped(GlobalFont,
                                           text.c_str(),
                                           color,
                                           (Uint32)line_length);
    SDL_assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    SDL_assert(texture);
    SDL_FreeSurface(surface);

    return Texture(texture, "", "", width, height);
}

// Loads a texture displaying an image, given a path to it.
Texture
LoadTextureImage(string path, string filename)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    surface = IMG_Load((path + filename).c_str());
    if(!surface)
        cout << "ERROR LoadTextureImage: couldn't load at " << filename << "\n";
    SDL_assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    SDL_assert(texture);
    SDL_FreeSurface(surface);

    return Texture(texture, path, filename, width, height);
}

// =================================== level data ===============================
string
LoadEntireFile(string filename)
{
    stringstream buffer;
    ifstream fp(filename);
    if(!fp.is_open())
    {
        cout << "ERROR LoadEntireFile(): File could not be opened: " << filename << "\n";
        return "";
    }
    buffer << fp.rdbuf();
    fp.close();

    return buffer.str();
}

// helper to split strings by a delimiter
vector<string>
split(const string &text, char sep)
{
    vector<string> tokens;
    size_t start = 0, end = 0;
    while((end = text.find(sep, start)) != string::npos)
    {
        if (end != start)
        {
            tokens.push_back(text.substr(start, end - start));
        }
        start = end + 1;
    }
    if (end != start)
       tokens.push_back(text.substr(start));

    return tokens;
}

// loads a level from a file.
Level
LoadLevel(string filename_in, const vector<shared_ptr<Unit>> &units,
          const vector<shared_ptr<Unit>> &party)
{
    string line;
    string type;
    string rest;
    vector<string> tokens;

    int mapRow = 0;

    Level level;
    level.name = filename_in;

    string buffer = LoadEntireFile(LEVELS_PATH + filename_in);
    vector<string> lines = split(buffer, '\n');

    for(string &line : lines)
    {
        if(line.empty())
            continue;

        type = line.substr(0, 3);
        rest = line.substr(4);
        tokens = split(rest, ' ');

        if(type == "ATL")
        {
            level.map.atlas = LoadTextureImage(TILESETS_PATH, rest);
        }
        else if(type == "MUS")
        {
            level.song = GetMusic(rest);
        }
        else if(type == "MAP")
        {
            for(int col = 0; col < MAP_WIDTH; ++col)
                level.map.tiles[col][mapRow] = TileTypeToTile((TileType)stoi(tokens[col]));

            ++mapRow;
        }
        else if(type == "UNT")
        {
            // NOTE: We go through party first. If we have any matches, we plop those down.
            // Otherwise, we'll grab them from the base units file.
            shared_ptr<Unit> unitCopy;
            for(const shared_ptr<Unit> &unit : party)
            {
                if(hash<string>{}(tokens[0]) == unit->ID())
                    unitCopy = make_shared<Unit>(*unit);
            }

            if(!unitCopy)
            {
                for(const shared_ptr<Unit> &unit : units)
                {
                    if(hash<string>{}(tokens[0]) == unit->ID())
                        unitCopy = make_shared<Unit>(*unit);
                }
            }

            SDL_assert(unitCopy);
            int col = stoi(tokens[1]);
            int row = stoi(tokens[2]);

            //level.AddCombatant(unitCopy, {col, row});
            // TODO: swap this?
            unitCopy->pos.col = col;
            unitCopy->pos.row = row;
            level.combatants.push_back(std::move(unitCopy));
            level.map.tiles[col][row].occupant = level.combatants.back().get();
        }
        else if(type == "COM")
        {
        }
        else
        {
            cout << "Warning LoadLevel: Unhandled line type: " << type << "\n";
        }
    }

	return level;
}

// loads units from a file. returns a vector of them.
vector<shared_ptr<Unit>>
LoadUnits(string filename_in)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	vector<shared_ptr<Unit>> units;

    ifstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());
    while(getline(fp, line))
    {
        if(!line.empty())
        {
            type = line.substr(0, 3);
            rest = line.substr(4);

            if(type == "UNT")
            {
                tokens = split(rest, '\t');

                units.push_back(make_shared<Unit>(
                    tokens[0],             // name
                    (Team)stoi(tokens[1]), // team
                    (ClassType)stoi(tokens[2]), // class
                    stoi(tokens[3]),       // health
                    stoi(tokens[4]),       // movement
                    Spritesheet(LoadTextureImage(SPRITES_PATH, tokens[5]),
                                32, ANIMATION_SPEED)
                ));
            }
        }
    }
    fp.close();

	return units;
}


// saves the units to a file.
void
SaveUnits(string filename_in, const vector<shared_ptr<Unit>> &units)
{
    ofstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());

    fp << "COM\t<name>\t<team>\t<class>\t<hp>\t<mov>\t<range>\t<sprite>\n";
    for(const shared_ptr<Unit> &unit : units)
    {
        fp << "UNT\t" << unit->name << "\t"
           << unit->team << "\t"
           << unit->cls.type << "\t"
           << unit->max_health << "\t"
           << unit->movement << "\t"

           << unit->sheet.texture.filename
           << "\n";
    }
    fp.close();
}


// saves a level to a file.
void
SaveLevel(string filename_in, const Level &level)
{
    ofstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());
    
    fp << "ATL " << level.map.atlas.filename << "\n\n";

    fp << "MUS " << level.song->name << "\n\n";

    // Save Map Data
    for(int row = 0; row < MAP_HEIGHT; ++row)
    {
        fp << "MAP ";
        for(int col = 0; col < MAP_WIDTH; ++col)
        {
            fp << level.map.tiles[col][row].type << " ";
        }
        fp << "\n";
    }
    fp << "\n";

    fp << "COM <name> <col> <row>\n";
    for(const shared_ptr<Unit> &unit : level.combatants)
    {
        fp << "UNT " << unit->name << " "
                     << unit->pos.col << " "
                     << unit->pos.row << " "
                     << "\n";
    }
    fp << "\n";
    fp.close();
}

#endif
