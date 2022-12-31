// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

#ifndef LOAD_H
#define LOAD_H

#include <fstream>
#include <iostream>

// ============================== loading data =================================
// Loads a Texture displaying the given text in the given color.
Texture
LoadTextureText(std::string text, SDL_Color color)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    assert(GlobalFont);
    surface = TTF_RenderText_Solid(GlobalFont, text.c_str(), color);
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

    cout << path + filename << "\n";
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
Level LoadLevel(string filename_in, const vector<unique_ptr<Unit>> &units)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	int mapRow = 0;

    Level level;

    ifstream fp;
    fp.open(filename_in);
    if(fp.is_open())
    {
        while(getline(fp, line))
        {
            if(!line.empty())
            {
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
				}
				else if(type == "MST")
				{
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
                    unique_ptr<Unit> unitCopy = make_unique<Unit>(*units[stoi(tokens[1])]);
                    int col = stoi(tokens[2]);
                    int row = stoi(tokens[3]);
                    unitCopy->col = col;
                    unitCopy->row = row;
                    level.combatants.push_back(move(unitCopy));
                    level.map.tiles[col][row].occupant = level.combatants.back().get();
				}
            }
        }
    }
    else
    {
        printf("LoadLevel ERROR\n");
    }
    fp.close();

	return level;
}

// loads units from a file. returns a vector of them.
vector<unique_ptr<Unit>> LoadUnits(string filename_in)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	vector<unique_ptr<Unit>> units;

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
                units.push_back(make_unique<Unit>(
                    tokens[0],									// name
                    SpriteSheet(LoadTextureImage(SPRITES_PATH, tokens[1]), 32, ANIMATION_SPEED), // path to texture
                    LoadTextureImage(PORTRAITS_PATH, tokens[2]), // portrait
                    stoi(tokens[3]),							// id
                    tokens[4] == "Ally" ? true : false,			// team
                    stoi(tokens[5]),							// movement
                    stoi(tokens[6]),							// hp
                    stoi(tokens[6]),							// max hp
                    stoi(tokens[7]),						    // short range
                    stoi(tokens[8]),						    // long range
                    stoi(tokens[9]),						    // attack
                    stoi(tokens[10]),						    // defense
                    stoi(tokens[11])						    // accuracy
                ));

                // TODO: Make the ID system generate an id in a more robust way.
                if(stoi(tokens[3]) > GlobalCurrentID)
                {
                    GlobalCurrentID = stoi(tokens[3]) + 1;
                }
            }
        }
    }
    fp.close();

	return units;
}

// ================================ saving ====================================

// saves the units to a file.
void
SaveUnits(string filename_in, const vector<unique_ptr<Unit>> &units)
{
    ofstream fp;
    fp.open(filename_in);
    assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Units\n";
    fp << "COM Date: November 2022\n\n";
    fp << "COM NOTE: Don't use tabs in here!\n\n";

    fp << "COM <UNT <name>  <texture> <portrait> <id> <team> <mov> <hp> <max> <short> <long> <atk> <def> <hit>>\n";
    for(const unique_ptr<Unit> &unit : units)
    {
        fp << "UNT " << unit->name << " "
                     << unit->sheet.texture.filename << " "
                     << unit->portrait.filename << " "
                     << unit->id << " "
                     << (unit->isAlly ? "Ally" : "Enemy") << " "
                     << unit->mov << " "
                     << unit->hp << " "
                     << unit->maxHp << " "
                     << unit->minRange << " "
                     << unit->maxRange << " "
                     << unit->attack << " "
                     << unit->defense << " "
                     << unit->accuracy << " "
                     << "\n";
    }
    fp.close();
}

// saves the level to a file.
void
SaveLevel(string filename_in, const Level &level)
{
    ofstream fp;
    fp.open(filename_in);
    assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Level\n";
    fp << "COM Date: November 2022\n\n";
    fp << "COM NOTE: Don't use tabs in here!\n\n";

    fp << "ATL " << level.map.atlas.filename << "\n\n";

    // Save Map Data
    fp << "WDT " << level.map.width << "\n";
    fp << "HGT " << level.map.height << "\n\n";
    fp << "MST 0\n";
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

    fp << "COM <UNT <name> <id> <col> <row>\n";
    // TODO: Separate allies and enemies when we save levels?
    for(const unique_ptr<Unit> &unit : level.combatants)
    {
        fp << "UNT " << unit->name << " "
                     << unit->id << " "
                     << unit->col << " "
                     << unit->row << " "
                     << "\n";
    }
    fp << "\n";
    fp.close();
}

#endif
