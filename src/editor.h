// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor

#ifndef EDITOR_H
#define EDITOR_H

#if DEV_MODE

void Audio(Level *level) {
    ImGui::Begin("Audio");
    {
        static float music_volume = DEFAULT_MUSIC_VOLUME;
        static float sfx_volume = DEFAULT_SFX_VOLUME;
        if(ImGui::SliderFloat("Music", &music_volume, 0.0f, 1.0f))
            SetMusicVolume(music_volume);
        if(ImGui::SliderFloat("SFX", &sfx_volume, 0.0f, 1.0f))
            SetSfxVolume(sfx_volume);
        if(ImGui::Button("Play"))
            level->song->Start();
        ImGui::SameLine();
        if(ImGui::Button("Pause"))
            level->song->Pause();
        ImGui::SameLine();
        if(ImGui::Button("Stop"))
            level->song->Stop();
    }
    ImGui::End();
}

static uint8_t selectedIndex = 0;
void UnitEditor(vector<shared_ptr<Equip>> *equipments,
                vector<shared_ptr<Unit>> *units)
{
    ImGui::Begin("unit editor");
    {
        /*
        if(ImGui::Button("create"))
        {
            units->push_back(make_shared<Unit>(
                string("DEFAULT_CHANGE"),
                TEAM_ENV,
                3,
                3,
                false,
                nullptr,
                nullptr,

                Spritesheet(LoadTextureImage(SPRITES_PATH, string(DEFAULT_SHEET)), 32, ANIMATION_SPEED)
            ));
        }
        ImGui::SameLine();
        if(ImGui::Button("destroy"))
        {
            units->erase(units->begin() + selectedIndex);
            if(selectedIndex > 0)
            {
                --selectedIndex;
            }
        }
        */

        for(int i = 0; i < units->size(); ++i)
        {
            char buffer[256];
            sprintf(buffer, "%d", i);
            if (ImGui::Button(buffer))
                selectedIndex = i;
            if(i % 7 || i == 0)
                ImGui::SameLine();
        }
        ImGui::NewLine();

        SDL_assert(selectedIndex < units->size());
        Unit *selected = (*units)[selectedIndex].get();

        ImGui::Text("%s | %zu", selected->name.c_str(), selected->ID());
        ImGui::InputText("unit name", &(selected->name));
        ImGui::SliderInt("health", &selected->max_health, 1, 8);
        ImGui::SliderInt("movement", &selected->movement, 1, 8);
        ImGui::Checkbox("fixed?", &selected->fixed);

        ImGui::Text("Items");
        if(selected->primary)
            ImGui::Text("Primary | %s", selected->primary->name.c_str());
        if(selected->secondary)
            ImGui::Text("Secondary | %s", selected->secondary->name.c_str());
        if(selected->healing)
            ImGui::Text("Healing | %s", selected->healing->name.c_str());

        static int index = 0;
        ImGui::SliderInt("index", &index, 0, 30);
        if(index < equipments->size())
        {
            ImGui::Text("%s", equipments->at(index)->name.c_str());
            if(ImGui::Button("Primary"))
            {
                if(selected->primary) // TODO: Shouldn't have to do this. the pointer should be null, and you can call delete on nullptr.
                    delete selected->primary;
                selected->primary = new Equip(*equipments->at(index));
            }
            ImGui::SameLine();
            if(ImGui::Button("Secondary"))
            {
                if(selected->secondary)
                    delete selected->secondary;
                selected->secondary = new Equip(*equipments->at(index));
            }
            ImGui::SameLine();
            if(ImGui::Button("Healing"))
            {
                if(selected->healing)
                    delete selected->healing;
                selected->healing = new Equip(*equipments->at(index));
            }

            ImGui::InputText("equip name", &(equipments->at(index)->name));

            ImGui::SliderInt("t", (int *)&(equipments->at(index)->type), 0, 10);
            ImGui::SameLine();
            ImGui::Text("%s", GetEquipmentString(equipments->at(index)->type).c_str());

            ImGui::SliderInt("c", (int *)&(equipments->at(index)->cls), 0, 10);
            ImGui::SameLine();
            ImGui::Text("%s", GetClassString(equipments->at(index)->cls).c_str());

            ImGui::SliderInt("p", (int *)&(equipments->at(index)->push), 0, 10);
            ImGui::SameLine();
            ImGui::Text("%s", GetPushString(equipments->at(index)->push).c_str());

            ImGui::SliderInt("m", (int *)&(equipments->at(index)->move), 0, 10);
            ImGui::SameLine();
            ImGui::Text("%s", GetMovementString(equipments->at(index)->move).c_str());

            ImGui::SliderInt("damage", &(equipments->at(index)->damage), 0, 3);
            ImGui::SliderInt("push damage", &(equipments->at(index)->push_damage), 0, 3);
            ImGui::SliderInt("self damage", &(equipments->at(index)->self_damage), 0, 3);
            ImGui::SliderInt("min range", &(equipments->at(index)->min_range), 0, 8);
            ImGui::SliderInt("max range", &(equipments->at(index)->max_range), 0, 8);
        }
        else
        {
            ImGui::Text("Invalid Equipment index");
        }
    }
    ImGui::End();
}

// Generate arbitrary debug paths
void 
GenerateDebugPaths(const Level &level, path *path_debug)
{
    static position start = {0, 0};
    static position end = {0, 0};
    ImGui::Text("From:");
    ImGui::SliderInt("fcol", &start.col, 0, 10);
    ImGui::SliderInt("frow", &start.row, 0, 10);
    ImGui::Text("To:");
    ImGui::SliderInt("dcol", &end.col, 0, 10);
    ImGui::SliderInt("drow", &end.row, 0, 10);
    if(ImGui::Button("from"))
    {
        *path_debug = GetPath(level.map, start, end, true);
    }
}

void
EditorPollForKeyboardInput(position *editor_cursor, int width, int height)
{
    position desired_move = *editor_cursor;
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
        desired_move = desired_move + position(-1, 0);
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
        desired_move = desired_move + position(1, 0);
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)))
        desired_move = desired_move + position(0, -1);
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
        desired_move = desired_move + position(0, 1);

    desired_move = clamp(desired_move, position(0, 0), position(width-1, height-1));
    *editor_cursor = desired_move;
}

void LevelEditor(Level *level, const vector<shared_ptr<Unit>> &units)
{
    static position editor_cursor = {0, 0};
    static path path_debug = {};
    EditorPollForKeyboardInput(&editor_cursor, MAP_WIDTH, MAP_HEIGHT);
    Tile *hover_tile = &level->map.tiles[editor_cursor.col][editor_cursor.row];

    ImGui::Begin("level editor");
    {
        ImGui::Text("%s", level->name.c_str());
        ImGui::Checkbox("spawning", &GlobalSpawning);
        ImGui::SliderInt("victory", &level->victory_turn, 1, 6);
        // =======================  Tile stuff  ================================
        ImGui::Text("Tiles:");
        if(ImGui::Button("plains"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_PLAIN);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("mountain"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_MOUNTAIN);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("forest"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_FOREST);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("water"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_WATER);
            hover_tile->occupant = tmp;
        }
        if(ImGui::Button("fort"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_FORT);
            hover_tile->occupant = tmp;
        }

        if(ImGui::Button("swamp"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_SWAMP);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("flame"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_FLAME);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("acid"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_ACID);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("desert"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_DESERT);
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("smoke"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GetTile(TILE_SMOKE);
            hover_tile->occupant = tmp;
        }



        ImGui::Text("Units:");
        if(ImGui::Button("add"))
        {
            if(hover_tile->occupant)
            {
                cout << "MISUSE: Cannot place unit there.\n";
            }
            else
            {
                level->combatants.push_back(make_shared<Unit>(*units[selectedIndex]));
                level->combatants.back()->pos.col = editor_cursor.col;
                level->combatants.back()->pos.row = editor_cursor.row;
                hover_tile->occupant = level->combatants.back().get();
            }
        }

        ImGui::SameLine();
        if(ImGui::Button("remove"))
        {
            Unit *map_ptr = hover_tile->occupant;
            if(map_ptr)
            {
                // NOTE: This is the biggest hack haha don't do this at home kids
                // Basically, it looks through the combatants and uses the
                // address of their pointers to determine which unit to remove.
                // Better solutions include:
                // * Using an ID system per level, allowing for fast lookups as well.
                // * Literally anything else.
                level->combatants.erase(
                        remove_if(level->combatants.begin(), level->combatants.end(),
                                [map_ptr](const shared_ptr<Unit> &u)
                                {
                                    return (map_ptr == u.get());
                                }),
                            level->combatants.end());

                hover_tile->occupant->should_die = true;
                hover_tile->occupant = nullptr;
            }
            else
            {
                cout << "MISUSE: No unit to delete.\n";
            }
        }

        if(hover_tile->occupant)
        {
            ImGui::Text("%s", hover_tile->occupant->name.c_str());

            if(ImGui::Button("Dmg"))
            {
                hover_tile->occupant->Damage(1);
            }
            ImGui::SameLine();
            if(ImGui::Button("Heal"))
            {
                hover_tile->occupant->Heal(1);
            }
        }

        //static bool showDebugPaths = false;
        /*
        ImGui::Checkbox("debug paths", &showDebugPaths);
        if(showDebugPaths)
            GenerateDebugPaths(*level, &path_debug);

        // Render overlays to the main target
        if(path_debug.size() > 0)
        {
            for(const position &p : path_debug)
            {
                RenderTileColor({p.col, p.row}, healColor);
            }
        }
        */

        RenderTileColor({editor_cursor.col, editor_cursor.row}, editorColor);
    }
    ImGui::End();
}

// Renders all imgui stuff.
// Contains static variables that might trip some stuff up, just a heads up.
void
EditorPass(
           vector<shared_ptr<Equip>> *equipments,
           vector<shared_ptr<Unit>> *units,
           const vector<shared_ptr<Unit>> &party,
           Level *level, const vector<string> &levels,
           Resolution *resolution)
{
    // Internal variables
    static char equip_filename[128] = INITIAL_EQUIPS;
    static char unit_filename[128] = INITIAL_UNITS;
    static char level_filename[128] = INITIAL_LEVEL;

    ImGui::Begin("emblem editor");
    {
        if(ImGui::Button("Load"))
        {
            resolution->attacks.clear();

            *equipments = LoadEquips(DATA_PATH + string(equip_filename));
            cout << "Units loaded: " << equip_filename << "\n";

            *units = LoadUnits(DATA_PATH + string(unit_filename), *equipments);
            cout << "Units loaded: " << unit_filename << "\n";

            *level = LoadLevel(level_filename, *units, party);
            cout << "Level loaded: " << level_filename << "\n";

            GoToAIPhase();
        }
        ImGui::SameLine();
        if(ImGui::Button("Save"))
        {
            SaveLevel(string(LEVELS_PATH) + level->name, *level);
            cout << "Level saved: " << level->name << "\n";
        }

        if(ImGui::Button("Save Equips"))
        {
            SaveEquips(string(DATA_PATH) + string(equip_filename), *equipments);
            cout << "Equips saved: " << equip_filename << "\n";
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Units"))
        {
            SaveUnits(string(DATA_PATH) + string(unit_filename), *units);
            cout << "Units saved: " << unit_filename << "\n";
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Level As"))
        {
            SaveLevel(string(LEVELS_PATH) + string(level_filename), *level);
            cout << "Level saved: " << level_filename << "\n";
        }

        if(ImGui::Button("Test Zone"))
        {
            resolution->attacks.clear();
            *level = LoadLevel("test.txt", *units, party);
            sprintf(level_filename, "test.txt");
            GoToAIPhase();
        }
        int wrap = 0;
        for(const string &s : levels)
        {
            if(ImGui::Button(s.c_str()))
            {
                resolution->attacks.clear();
                *level = LoadLevel(s, *units, party);
                sprintf(level_filename, "%s", s.c_str());
                GoToAIPhase();
            }
            if(++wrap % 4)
                ImGui::SameLine();
        }

        ImGui::InputText("equip", equip_filename, IM_ARRAYSIZE(equip_filename));
        ImGui::InputText("units", unit_filename, IM_ARRAYSIZE(unit_filename));
        ImGui::InputText("level", level_filename, IM_ARRAYSIZE(level_filename));
    }
    ImGui::End();

    // Sub-Editors
    UnitEditor(equipments, units);
    LevelEditor(level, *units);
    Audio(level);

    /*
    // debug
    ImGui::ShowStyleEditor();
    ImGui::ShowDemoWindow();
    */
}
#endif

#endif
