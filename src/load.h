// Author: Alex Hartford
// Program: Emblem
// File: Load

#ifndef LOAD_H
#define LOAD_H

#include <fstream>
#include <vector>

// ============================== loading data =================================
Conversation
LoadConversation(string path, string filename,
                 const Unit &one, const Unit &two)
{
    Conversation conversation = {one, two};

    string line;
	string type;
	string rest;

    ifstream fp;
    fp.open(path + filename);
    if(!fp.is_open())
        assert(!"ERROR LoadConversation: File could not be opened.\n");

    while(getline(fp, line))
    {
        if(line.empty())
            continue;

        type = line.substr(0, 3);
        rest = line.substr(4);

        if(type == "ONE")
        {
            conversation.prose.push_back(sentence(SPEAKER_ONE, rest));
        }
        else if(type == "TWO")
        {
            conversation.prose.push_back(sentence(SPEAKER_TWO, rest));
        }
        else if(type == "COM")
        {
        }
        else
        {
            cout << "WARN LoadConversation: Unrecognized line type in " << filename << ".\n";
        }
    }
    fp.close();
    conversation.ReloadTextures();

    return conversation;
}

// Loads a Texture displaying the given text in the given color.
// Now with wrapping, set by the line_length parameter
Texture
LoadTextureText(string text, SDL_Color color, int line_length)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    assert(GlobalFont);
    surface = TTF_RenderText_Solid_Wrapped(GlobalFont,
                                           text.c_str(),
                                           color,
                                           (Uint32)line_length);
    assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    assert(texture);
    SDL_FreeSurface(surface);

    return Texture(texture, "", "", width, height);
}

// Loads a texture displaying an image, given a path to it.
Texture
LoadTextureImage(string path, string filename)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    //cout << path + filename << "\n";
    surface = IMG_Load((path + filename).c_str());
    assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    assert(texture);
    SDL_FreeSurface(surface);

    return Texture(texture, path, filename, width, height);
}

// =================================== level data ===============================
// helper to split strings by a delimiter
vector<string> split(const string &text, char sep) {
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != string::npos) {
        if (end != start) {
          tokens.push_back(text.substr(start, end - start));
        }
        start = end + 1;
    }
    if (end != start) {
       tokens.push_back(text.substr(start));
    }
    return tokens;
}

// loads a level from a file.
Level
LoadLevel(string filename_in, const vector<shared_ptr<Unit>> &units)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	int mapRow = 0;

    Level level;

    ifstream fp;
    fp.open(filename_in);
    if(!fp.is_open())
        assert(!"ERROR LoadLevel: File could not be opened!\n");

    while(getline(fp, line))
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
        else if(type == "WDT")
        {
            level.map.width = stoi(rest);
        }
        else if(type == "HGT")
        {
            level.map.height = stoi(rest);
            level.map.tiles.resize(level.map.width, vector<Tile>(level.map.height));
            for(int col = 0; col < level.map.width; ++col)
            {
                for(int row = 0; row < level.map.height; ++row)
                {
                    level.map.tiles[col][row] = {};
                }
            }
        }
        else if(type == "MAP")
        {
            for(int col = 0; col < level.map.width; ++col)
            {
                switch(stoi(tokens[col]))
                {
                    case(FLOOR):
                    {
                        level.map.tiles[col][mapRow] = 
                            FLOOR_TILE;
                    } break;
                    case(WALL):
                    {
                        level.map.tiles[col][mapRow] = 
                            WALL_TILE;
                    } break;
                    case(FOREST):
                    {
                        level.map.tiles[col][mapRow] = 
                            FOREST_TILE;
                    } break;
                    case(DESERT):
                    {
                        level.map.tiles[col][mapRow] = 
                            DESERT_TILE;
                    } break;
                    case(OBJECTIVE):
                    {
                        level.map.tiles[col][mapRow] = 
                            OBJECTIVE_TILE;
                    } break;
                    case(SPAWN):
                    {
                        level.map.tiles[col][mapRow] = 
                            SPAWN_TILE;
                    } break;
                    default:
                    {
                        cout << "ERROR: Unhandled tile type in loader!\n";
                    } break;
                }
            }
            ++mapRow;
        }
        else if(type == "UNT")
        {
            shared_ptr<Unit> unitCopy;
            for(const shared_ptr<Unit> &unit : units)
            {
                if(hash<string>{}(tokens[0]) == unit->ID())
                {
                    unitCopy = make_shared<Unit>(*unit);
                }
            }
            assert(unitCopy);
            int col = stoi(tokens[1]);
            int row = stoi(tokens[2]);

            unitCopy->pos.col = col;
            unitCopy->pos.row = row;
            unitCopy->ai_behavior = (AIBehavior)stoi(tokens[3]);
            level.combatants.push_back(move(unitCopy));
            level.map.tiles[col][row].occupant = level.combatants.back().get();
        }
    }
    fp.close();

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
    assert(fp.is_open());
    while(getline(fp, line))
    {
        if(!line.empty())
        {
            type = line.substr(0, 3);
            rest = line.substr(4);

            if(type == "UNT")
            {
                tokens = split(rest, ' ');
                units.push_back(make_shared<Unit>(
                    tokens[0],									// name
                    Spritesheet(LoadTextureImage(SPRITES_PATH, tokens[1]), 32, ANIMATION_SPEED), // path to texture
                    LoadTextureImage(FULLS_PATH, tokens[2]),// portrait
                    tokens[3] == "Ally" ? true : false,			// team
                    stoi(tokens[4]),							// movement
                    stoi(tokens[5]),							// health
                    stoi(tokens[5]),							// max health
                    stoi(tokens[6]),							// attack
                    stoi(tokens[7]),							// attack
                    stoi(tokens[8]),							// defense
                    stoi(tokens[9]),						    // accuracy
                    stoi(tokens[10]),						    // avoid
                    stoi(tokens[11]),						    // crit
                    stoi(tokens[12]),						    // short range
                    stoi(tokens[13]),						    // long range
                    (AIBehavior)stoi(tokens[14])                // ai behavior
                ));
            }
        }
    }
    fp.close();

	return units;
}

// ================================ saving ====================================

// saves the units to a file.
void
SaveUnits(string filename_in, const vector<shared_ptr<Unit>> &units)
{
    ofstream fp;
    fp.open(filename_in);
    assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Units\n\n";

    fp << "COM <UNT <name> <texture> <portrait> <team> <mov> <hp> <atk> <abi> <def> <acc> <avo> <crit> <short> <long> <ai>>\n";
    for(const shared_ptr<Unit> &unit : units)
    {
        fp << "UNT " << unit->name << " "
                     << unit->sheet.texture.filename << " "
                     << unit->portrait.filename << " "
                     << (unit->is_ally ? "Ally" : "Enemy") << " "
                     << unit->movement << " "
                     << unit->max_health << " "
                     << unit->attack << " "
                     << unit->ability << " "
                     << unit->defense << " "
                     << unit->accuracy << " "
                     << unit->avoid << " "
                     << unit->crit << " "
                     << unit->min_range << " "
                     << unit->max_range << " "
                     << unit->ai_behavior << " "
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
    assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Level\n\n";

    fp << "ATL " << level.map.atlas.filename << "\n\n";

    // Save Map Data
    fp << "WDT " << level.map.width << "\n";
    fp << "HGT " << level.map.height << "\n";
    for(int row = 0; row < level.map.height; ++row)
    {
        fp << "MAP ";
        for(int col = 0; col < level.map.width; ++col)
        {
            fp << level.map.tiles[col][row].type << " ";
        }
        fp << "\n";
    }
    fp << "\n";

    fp << "COM <UNT <name> <col> <row> <ai>\n";
    for(const shared_ptr<Unit> &unit : level.combatants)
    {
        fp << "UNT " << unit->name << " "
                     << unit->pos.col << " "
                     << unit->pos.row << " "
                     << unit->ai_behavior << " "
                     << "\n";
    }
    fp << "\n";
    fp.close();
}

#endif
