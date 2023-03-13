// Author: Alex Hartford
// Program: Emblem
// File: Load

#ifndef LOAD_H
#define LOAD_H

#include <fstream>
#include <sstream>
#include <vector>

// ============================== loading data =================================
// Loads a Texture displaying the given text in the given color.
// Now with wrapping, set by the line_length parameter
Texture
LoadTextureText(string text, SDL_Color color, int line_length, bool small)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    SDL_assert(GlobalFont);
    surface = TTF_RenderText_Solid_Wrapped((small ? GlobalFontSmall : GlobalFont),
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
        else if(type == "VIC")
        {
            level.victory_turn = stoi(rest);
        }
        else if(type == "MAP")
        {
            for(int col = 0; col < MAP_WIDTH; ++col)
                level.map.tiles[col][mapRow] = GetTile((TileType)stoi(tokens[col]));

            ++mapRow;
        }
        else if(type == "UNT")
        {
            shared_ptr<Unit> unitCopy;
            for(const shared_ptr<Unit> &unit : units)
            {
                if(hash<string>{}(tokens[0]) == unit->ID())
                    unitCopy = make_shared<Unit>(*unit);
            }

            SDL_assert(unitCopy);
            int col = stoi(tokens[1]);
            int row = stoi(tokens[2]);

            if(unitCopy->IsAlly())
                level.to_warp.push_back(unitCopy);
            else
                level.AddCombatant(unitCopy, {col, row});
        }
        else if(type == "SPW")
        {
            shared_ptr<Unit> unitCopy;

            for(const shared_ptr<Unit> &unit : units)
            {
                if(hash<string>{}(tokens[0]) == unit->ID())
                    unitCopy = make_shared<Unit>(*unit);
            }

            SDL_assert(unitCopy);

            level.spawner.pool.push_back(std::move(unitCopy));
        }
        else if(type == "COM")
        {
        }
        else
        {
            cout << "Warning LoadLevel: Unhandled line type: " << type << "\n";
        }
    }

    for(shared_ptr<Unit> party_member : party)
    {
        party_member->is_exhausted = false;
        level.to_warp.push_back(make_shared<Unit>(*party_member));
    }

	return level;
}

// loads units from a file. returns a vector of them.
vector<shared_ptr<Unit>>
LoadUnits(string filename_in, const vector<shared_ptr<Equip>> &equipments)
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
                    stoi(tokens[2]),       // health
                    stoi(tokens[3]),       // movement
                    (bool)stoi(tokens[4]), // fixed?
                    tokens[5] == " " ? nullptr : new Equip(*GetEquipByName(equipments, tokens[5])),
                    tokens[6] == " " ? nullptr : new Equip(*GetEquipByName(equipments, tokens[6])),
                    tokens[7] == " " ? nullptr : new Equip(*GetEquipByName(equipments, tokens[7])),

                    (EffectType)stoi(tokens[8]), // passive

                    Spritesheet(LoadTextureImage(SPRITES_PATH, tokens[9]), // sprite
                                32)
                ));
            }
        }
    }
    fp.close();

	return units;
}

// loads equipments from a file. returns a vector of them.
vector<shared_ptr<Equip>>
LoadEquips(string filename_in)
{
    string line;
	string type;
	string rest;
	vector<string> tokens;

	vector<shared_ptr<Equip>> equipments;

    ifstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());

    while(getline(fp, line))
    {
        if(!line.empty())
        {
            type = line.substr(0, 3);
            rest = line.substr(4);

            if(type == "WEA")
            {
                tokens = split(rest, '\t');
                equipments.push_back(make_shared<Equip>(
                    tokens[0],						// name
                    (EquipmentType)stoi(tokens[1]), // type
                    (ClassType)stoi(tokens[2]),     // class type
                    (PushType)stoi(tokens[3]),      // push type
                    (MovementType)stoi(tokens[4]),  // movement type
                    (EffectType)stoi(tokens[5]),    // movement type
                    stoi(tokens[6]),                // damage
                    stoi(tokens[7]),                // push damage
                    stoi(tokens[8]),                // self damage
                    stoi(tokens[9]),                // min range
                    stoi(tokens[10])                // max range
                ));
            }
        }
    }
    fp.close();

	return equipments;
}

// ================================ saving ====================================
// saves the units to a file.
void
SaveEquips(string filename_in, const vector<shared_ptr<Equip>> &equipments)
{
    ofstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());
    
    fp << "COM\t<name>\t<type>\t<class>\t<push>\t<move>\t<effec>\t<dmg>\t<p_dmg>\t<s_dmg>\t<min>\t<max>\n";
    for(const shared_ptr<Equip> &equip : equipments)
    {
        fp << "WEA\t"
           << equip->name << "\t"
           << equip->type << "\t"
           << equip->cls << "\t"
           << equip->push << "\t"
           << equip->move << "\t"
           << equip->effect << "\t"

           << equip->damage << "\t"
           << equip->push_damage << "\t"
           << equip->self_damage << "\t"
           << equip->min_range << "\t"
           << equip->max_range << "\t"
           << "\n";
    }
    fp.close();
}

// saves the units to a file.
void
SaveUnits(string filename_in, const vector<shared_ptr<Unit>> &units)
{
    ofstream fp;
    fp.open(filename_in);
    SDL_assert(fp.is_open());
    
    fp << "COM\t<name>\t<team>\t<hp>\t<mov>\t<fixed>\t<prim>\t<secnd>\t<util>\t<e1>\t<e2>\t<sprite>\n";
    for(const shared_ptr<Unit> &unit : units)
    {
        fp << "UNT\t" << unit->name << "\t"
           << unit->team << "\t"
           << unit->max_health << "\t"
           << unit->movement << "\t"
           << unit->fixed << "\t"

           << (unit->primary ? unit->primary->name : " ") << "\t"
           << (unit->secondary ? unit->secondary->name : " ") << "\t"
           << (unit->utility ? unit->utility->name : " ") << "\t"

           << unit->passive.type << "\t"

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

    fp << "VIC " << level.victory_turn << "\n\n";

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

    for(const shared_ptr<Unit> &unit : level.spawner.pool)
    {
        fp << "SPW " << unit->name << " "
                     << "\n";
    }
    fp << "\n";

    fp.close();
}

#endif
