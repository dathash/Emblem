// Author: Alex Hartford
// Program: Emblem
// File: Load

#ifndef LOAD_H
#define LOAD_H

#include <fstream>
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
Conversation
LoadConversation(string path, string filename,
                 const vector<shared_ptr<Unit>> units,
                 const position &pos_in = position(-1, -1))
{
    Conversation conversation = {};
    conversation.filename = filename;
    conversation.pos = pos_in;

    string line;
    string type;
    string rest;

    Expression one_expression = EXPR_NEUTRAL;
    Expression two_expression = EXPR_NEUTRAL;
    Expression three_expression = EXPR_NEUTRAL;
    Expression four_expression = EXPR_NEUTRAL;
    ConversationEvent conversation_event = CONV_NONE;

    ifstream fp;
    fp.open(path + filename);
    if(!fp.is_open())
    {
        cout << path << " " << filename << "\n";
        SDL_assert(!"ERROR LoadConversation: File could not be opened.\n");
    }

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
        else if(type == "SP3")
        {
            conversation.three = GetUnitByName(units, rest);
        }
        else if(type == "SP4")
        {
            conversation.four = GetUnitByName(units, rest);
        }
        else if(type == "EX1")
        {
            one_expression = GetExpressionFromString(rest);
        }
        else if(type == "EX2")
        {
            two_expression = GetExpressionFromString(rest);
        }
        else if(type == "EX3")
        {
            three_expression = GetExpressionFromString(rest);
        }
        else if(type == "EX4")
        {
            four_expression = GetExpressionFromString(rest);
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
        else if(type == "THR")
        {
            conversation.prose.push_back({SPEAKER_THREE, rest, three_expression, conversation_event});
        }
        else if(type == "FOU")
        {
            conversation.prose.push_back({SPEAKER_FOUR, rest, four_expression, conversation_event});
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

    //SDL_assert(conversation.one && conversation.two);

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

    surface = IMG_Load((path + filename).c_str());
    SDL_assert(surface);
    if(!surface)
        cout << "ERROR: " << filename << "\n";
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

    ifstream fp;
    fp.open(LEVELS_PATH + filename_in);

    if(!fp.is_open())
        SDL_assert(!"ERROR LoadLevel: File could not be opened!\n");

    while(getline(fp, line))
    {
        if(line.empty())
            continue;

        type = line.substr(0, 3);
        rest = line.substr(4);
        tokens = split(rest, ' ');

        if(type == "OBJ")
        {
            level.objective = (Objective)stoi(rest);
        }
        else if(type == "ATL")
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
        else if(type == "CUT")
        {
            level.conversations.cutscenes.push_back(cutscene(stoi(tokens[0]),
                        LoadConversation(CUTSCENES_PATH, tokens[1], units)));
        }
        else if(type == "VIL")
        {
            int col = stoi(tokens[0]);
            int row = stoi(tokens[1]);

            level.conversations.villages.push_back(
                    LoadConversation(VILLAGES_PATH, tokens[2], units, position(col, row)));
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

            unitCopy->pos.col = col;
            unitCopy->pos.row = row;
            unitCopy->ai_behavior = (AIBehavior)stoi(tokens[3]);
            unitCopy->is_boss = (bool)stoi(tokens[4]);
            unitCopy->arrival = stoi(tokens[5]);
            if(unitCopy->arrival > 0)
            {
                level.bench.push_back(move(unitCopy));
            }
            else
            {
                level.combatants.push_back(move(unitCopy));
                level.map.tiles[col][row].occupant = level.combatants.back().get();
            }
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
                tokens = split(rest, '\t');
                units.push_back(make_shared<Unit>(
                    tokens[0],									// name
                    tokens[1] == "Ally" ? true : false,			// team

                    // Bases
                    stoi(tokens[2]),							// movement
                    stoi(tokens[3]),							// health
                    stoi(tokens[3]),							// max health
                    stoi(tokens[4]),							// attack
                    stoi(tokens[5]),							// aptitude
                    stoi(tokens[6]),							// defense
                    stoi(tokens[7]),							// speed
                    stoi(tokens[8]),						    // skill
                    stoi(tokens[9]),						    // short range
                    stoi(tokens[10]),						    // long range

                    stoi(tokens[11]),						    // level

                    (Ability)stoi(tokens[12]),				    // ability

                    (AIBehavior)stoi(tokens[13]),               // ai behavior

                    stoi(tokens[14]),                           // xp value

                    // Growths
                    stoi(tokens[15]),                           // health
                    stoi(tokens[16]),                           // attack
                    stoi(tokens[17]),                           // aptitude
                    stoi(tokens[18]),                           // defense
                    stoi(tokens[19]),                           // speed
                    stoi(tokens[20]),                           // skill

                    // Textures
                    Spritesheet(LoadTextureImage(SPRITES_PATH, tokens[21]), 32, ANIMATION_SPEED), // path to texture
                    LoadTextureImage(FULLS_PATH, tokens[22]),   // neutral
                    LoadTextureImage(FULLS_PATH, tokens[23]),   // happy
                    LoadTextureImage(FULLS_PATH, tokens[24]),   // angry
                    LoadTextureImage(FULLS_PATH, tokens[25]),   // wince
                    tokens[26]                                  // valediction
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

    fp << "COM\t<name>\t<team>\t<mov>\t<hp>\t<atk>\t<apt>\t<def>\t<spd>\t<skl>\t<short>\t<long>\t<level>\t<abi>\t<ai>\t<xpv>\t<ghp>\t<gat>\t<gap>\t<gdf>\t<gsp>\t<gsk>\t<texture>\t<neutral>\t<happy>\t<angry>\t<wince>\n";
    for(const shared_ptr<Unit> &unit : units)
    {
        fp << "UNT " << unit->name << "\t"
                     << (unit->is_ally ? "Ally" : "Enemy") << "\t"
                     << unit->movement << "\t"
                     << unit->max_health << "\t"
                     << unit->attack << "\t"
                     << unit->aptitude << "\t"
                     << unit->defense << "\t"
                     << unit->speed << "\t"
                     << unit->skill << "\t"
                     << unit->min_range << "\t"
                     << unit->max_range << "\t"
                     << unit->level << "\t"
                     << unit->ability << "\t"
                     << unit->ai_behavior << "\t"
                     << unit->xp_value << "\t"

                     << unit->growths.health << "\t"
                     << unit->growths.attack << "\t"
                     << unit->growths.aptitude << "\t"
                     << unit->growths.defense << "\t"
                     << unit->growths.speed << "\t"
                     << unit->growths.skill << "\t"

                     << unit->sheet.texture.filename << "\t"
                     << unit->neutral.filename << "\t"
                     << unit->happy.filename << "\t"
                     << unit->angry.filename << "\t"
                     << unit->wince.filename << "\t"
                     << unit->valediction
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

    fp << "OBJ " << level.objective << "\n\n";

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

    for(const Conversation &conv : level.conversations.villages)
    {
        fp << "VIL " << conv.pos.col << " " << conv.pos.row << " " << conv.filename << "\n";
    }
    fp << "\n";

    for(const Conversation &conv : level.conversations.list)
    {
        fp << "CNV " << conv.filename << "\n";
    }
    fp << "\n";

    for(const cutscene &cs : level.conversations.cutscenes)
    {
        fp << "CUT " << cs.first << " " << cs.second.filename << "\n";
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

    fp << "COM <UNT <name> <col> <row> <ai> <boss> <start>>\n";
    for(const shared_ptr<Unit> &unit : level.combatants)
    {
        fp << "UNT " << unit->name << " "
                     << unit->pos.col << " "
                     << unit->pos.row << " "
                     << unit->ai_behavior << " "
                     << unit->is_boss << " "
                     << unit->arrival << " "
                     << "\n";
    }
    fp << "\n";

    for(const shared_ptr<Unit> &unit : level.bench)
    {
        fp << "UNT " << unit->name << " "
                     << unit->pos.col << " "
                     << unit->pos.row << " "
                     << unit->ai_behavior << " "
                     << unit->is_boss << " "
                     << unit->arrival << " "
                     << "\n";
    }
    fp << "\n";
    fp.close();
}

#endif
