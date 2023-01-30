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
                 const vector<shared_ptr<Unit>> units)
{
    Conversation conversation = {};
    conversation.filename = filename;

    string line;
    string type;
    string rest;

    Expression one_expression = EXPR_NEUTRAL;
    Expression two_expression = EXPR_NEUTRAL;
    ConversationEvent conversation_event = CONV_NONE;

    ifstream fp;
    fp.open(path + filename);
    if(!fp.is_open())
        SDL_assert(!"ERROR LoadConversation: File could not be opened.\n");

    while(getline(fp, line))
    {
        if(line.empty())
            continue;

        type = line.substr(0, 3);
        rest = line.substr(4);

        if(type == "MUS")
        {
            conversation.song = GetMusic(rest);
        }
        else if(type == "SP1")
        {
            conversation.one = GetUnitByName(units, rest);
        }
        else if(type == "SP2")
        {
            conversation.two = GetUnitByName(units, rest);
        }
        else if(type == "EX1")
        {
            one_expression = GetExpressionFromString(rest);
        }
        else if(type == "EX2")
        {
            two_expression = GetExpressionFromString(rest);
        }
        else if(type == "EVE")
        {
            conversation_event = GetConversationEventFromString(rest);
        }
        else if(type == "ONE")
        {
            conversation.prose.push_back({SPEAKER_ONE, rest, one_expression, conversation_event});
        }
        else if(type == "TWO")
        {
            conversation.prose.push_back({SPEAKER_TWO, rest, two_expression, conversation_event});
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

    SDL_assert(conversation.one && conversation.two);

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

    //cout << path + filename << "\n";
    surface = IMG_Load((path + filename).c_str());
    SDL_assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    SDL_assert(texture);
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
    level.name = filename_in;

    ifstream fp;
    fp.open(DATA_PATH + filename_in);
    if(!fp.is_open())
        SDL_assert(!"ERROR LoadLevel: File could not be opened!\n");

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
        else if(type == "PRE")
        {
            level.conversations.prelude = 
                    LoadConversation(PRELUDES_PATH, rest, units);
        }
        else if(type == "MID")
        {
            level.conversations.mid_battle.push_back(
                    LoadConversation(CONVERSATIONS_PATH, rest, units));
        }
        else if(type == "CNV")
        {
            level.conversations.list.push_back(
                    LoadConversation(CONVERSATIONS_PATH, rest, units));
        }
        else if(type == "MUS")
        {
            level.song = GetMusic(rest);
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
                    case(SWAMP):
                    {
                        level.map.tiles[col][mapRow] = 
                            SWAMP_TILE;
                    } break;
                    case(FORT):
                    {
                        level.map.tiles[col][mapRow] = 
                            FORT_TILE;
                    } break;
                    case(GOAL):
                    {
                        level.map.tiles[col][mapRow] = 
                            GOAL_TILE;
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
            SDL_assert(unitCopy);
            int col = stoi(tokens[1]);
            int row = stoi(tokens[2]);

            unitCopy->pos.col = col;
            unitCopy->pos.row = row;
            unitCopy->ai_behavior = (AIBehavior)stoi(tokens[3]);
            level.combatants.push_back(move(unitCopy));
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
    SDL_assert(fp.is_open());
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
                    LoadTextureImage(FULLS_PATH, tokens[2]),    // neutral
                    LoadTextureImage(FULLS_PATH, tokens[3]),    // happy
                    LoadTextureImage(FULLS_PATH, tokens[4]),    // angry
                    LoadTextureImage(FULLS_PATH, tokens[5]),    // wince
                    tokens[6] == "Ally" ? true : false,			// team
                    stoi(tokens[7]),							// movement
                    stoi(tokens[8]),							// health
                    stoi(tokens[8]),							// max health
                    stoi(tokens[9]),							// attack
                    stoi(tokens[10]),							// aptitude
                    stoi(tokens[11]),							// defense
                    stoi(tokens[12]),							// speed
                    stoi(tokens[13]),						    // accuracy
                    stoi(tokens[14]),						    // avoid
                    stoi(tokens[15]),						    // crit
                    stoi(tokens[16]),						    // short range
                    stoi(tokens[17]),						    // long range
                    (Ability)stoi(tokens[18]),				    // ability
                    (AIBehavior)stoi(tokens[19])                // ai behavior
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
    SDL_assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Units\n\n";

    fp << "COM <UNT <name> <texture> <portrait> <team> <mov> <hp> <atk> <apt> <def> <spd> <acc> <avo> <crit> <short> <long> <abi> <ai>>\n";
    for(const shared_ptr<Unit> &unit : units)
    {
        fp << "UNT " << unit->name << " "
                     << unit->sheet.texture.filename << " "
                     << unit->neutral.filename << " "
                     << unit->happy.filename << " "
                     << unit->angry.filename << " "
                     << unit->wince.filename << " "
                     << (unit->is_ally ? "Ally" : "Enemy") << " "
                     << unit->movement << " "
                     << unit->max_health << " "
                     << unit->attack << " "
                     << unit->aptitude << " "
                     << unit->defense << " "
                     << unit->speed << " "
                     << unit->accuracy << " "
                     << unit->avoid << " "
                     << unit->crit << " "
                     << unit->min_range << " "
                     << unit->max_range << " "
                     << unit->ability << " "
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
    SDL_assert(fp.is_open());
    
    fp << "COM Author: Alex Hartford\n";
    fp << "COM Program: Emblem\n";
    fp << "COM File: Level\n\n";

    fp << "ATL " << level.map.atlas.filename << "\n\n";

    if(level.conversations.prelude.one)
    {
        fp << "PRE " << level.conversations.prelude.filename << "\n";
        fp << "\n";
    }
    for(const Conversation &conv : level.conversations.mid_battle)
    {
        fp << "MID " << conv.filename << "\n";
    }
    fp << "\n";
    for(const Conversation &conv : level.conversations.list)
    {
        fp << "CNV " << conv.filename << "\n";
    }
    fp << "\n";

    fp << "MUS " << level.song->name << "\n\n";

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
