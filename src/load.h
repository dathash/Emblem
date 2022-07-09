// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

#ifndef LOAD_H
#define LOAD_H

// ============================== loading data =================================
// Loads a Texture displaying the given text in the given color.
unique_ptr<Texture>
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

    return make_unique<Texture>(texture, "", width, height);
}

// Loads a texture displaying an image, given a path to it.
shared_ptr<Texture>
LoadTextureImage(std::string path)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    surface = IMG_Load(path.c_str());
    assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    assert(texture);
    SDL_FreeSurface(surface);

    return make_shared<Texture>(texture, path, width, height);
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

// loads a level from a file. returns a new tilemap.
shared_ptr<Level> LoadLevel(string filename_in, const vector<shared_ptr<Unit>> &units)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	int mapRow = 0;

    shared_ptr<Level> level = make_shared<Level>();

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

				if(type == "MAP")
				{
					for(int col = 0; col < MAP_SIZE; ++col)
					{
						switch(stoi(tokens[col]))
						{
							case(0):
							{
								level->map.tiles[col][mapRow].type = FLOOR;
								level->map.tiles[col][mapRow].penalty = 1;
							} break;
							case(1):
							{
								level->map.tiles[col][mapRow].type = WALL;
								level->map.tiles[col][mapRow].penalty = 100;
							} break;
							default:
							{
							} break;
						}
					}
					++mapRow;
				}
				else if(type == "UNT")
				{
                    shared_ptr<Unit> unitBase = units[stoi(tokens[1])];
                    // TODO: Figure out how to not do this. 
                    shared_ptr<Unit> unitCopy = make_shared<Unit>(
                                                                  unitBase->name,
                                                                  LoadTextureImage(unitBase->sheet->texture->path),
                                                                  unitBase->id,
                                                                  unitBase->isAlly,
                                                                  unitBase->mov,
                                                                  unitBase->hp,
                                                                  unitBase->maxHp,
                                                                  unitBase->minRange,
                                                                  unitBase->maxRange,
                                                                  unitBase->attack,
                                                                  unitBase->defense,
                                                                  unitBase->accuracy
                                                                  );
                    int col = stoi(tokens[2]);
                    int row = stoi(tokens[3]);
                    unitCopy->col = col;
                    unitCopy->row = row;
                    if(unitCopy->isAlly)
                    {
                        level->allies.push_back(unitCopy);
                    }
                    else
                    {
                        level->enemies.push_back(unitCopy);
                    }
					level->map.tiles[col][row].occupant = unitCopy;
					level->map.tiles[col][row].occupied = true;
				}
            }
        }
    }
    else
    {
        cout << "LoadLevel ERROR\n";
    }
    fp.close();

	return level;
}

// loads the characters for the game from a file. returns a vector of them.
vector<shared_ptr<Unit>> LoadCharacters(string filename_in)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	vector<shared_ptr<Unit>> units;

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

				if(type == "UNT")
				{
					tokens = split(rest, ' ');
					units.push_back(make_shared<Unit>(
						tokens[0],									// name
						LoadTextureImage("../assets/sprites/" + tokens[1]), // path to texture
						stoi(tokens[2]),							// id
						tokens[3] == "Ally" ? true : false,			// team
						stoi(tokens[4]),							// movement
						stoi(tokens[5]),							// hp
						stoi(tokens[6]),							// max hp
						stoi(tokens[7]),						    // short range
						stoi(tokens[8]),						    // long range
						stoi(tokens[9]),						    // attack
						stoi(tokens[10]),						    // defense
						stoi(tokens[11])						    // accuracy
					));
				}
            }
        }
    }
    else
    {
        cout << "LoadCharacters ERROR\n";
    }
    fp.close();

	return units;
}

#endif
