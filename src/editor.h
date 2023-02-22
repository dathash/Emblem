// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor

#ifndef EDITOR_H
#define EDITOR_H

#if DEV_MODE

void
Audio(Level *level)
{
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
void
UnitEditor(vector<shared_ptr<Unit>> *units)
{
    ImGui::Begin("unit editor");
    {
        if(ImGui::Button("create"))
        {
            units->push_back(make_shared<Unit>(
                string("DEFAULT_CHANGE"),
                TEAM_ENV,
                3,
                3,
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
        ImGui::SliderInt("movement", &selected->movement, 0, 5);
    }
    ImGui::End();
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
        // =======================  Tile stuff  ================================
        ImGui::Text("Tiles:");
        if(ImGui::Button("none"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = FLOOR_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("wall"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = WALL_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("cover"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = FOREST_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("slow"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = SWAMP_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("goal"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = GOAL_TILE;
            hover_tile->occupant = tmp;
        }

        if(ImGui::Button("fort"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = FORT_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("village"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = VILLAGE_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("chest"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = CHEST_TILE;
            hover_tile->occupant = tmp;
        }


        ImGui::Text("Units:");
        if(ImGui::Button("add"))
        {
            if(!(hover_tile->occupant ||
                 hover_tile->type == WALL))
            {
                level->combatants.push_back(make_shared<Unit>(*units[selectedIndex]));
                level->combatants.back()->pos.col = editor_cursor.col;
                level->combatants.back()->pos.row = editor_cursor.row;
                hover_tile->occupant = level->combatants.back().get();
            }
            else
            {
                cout << "MISUSE: Cannot place unit there.\n";
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
            ImGui::Text("Over unit.");

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

        RenderTileColor({editor_cursor.col, editor_cursor.row}, editorColor);
    }
    ImGui::End();
}

// Renders all imgui stuff.
// Contains static variables that might trip some stuff up, just a heads up.
void
EditorPass(
           vector<shared_ptr<Unit>> *units,
           const vector<shared_ptr<Unit>> &party,
           Level *level, const vector<string> &levels,
           Resolution *resolution)
{
    // Internal variables
    static char unit_filename[128] = INITIAL_UNITS;
    static char level_filename[128] = INITIAL_LEVEL;

    ImGui::Begin("emblem editor");
    {
        if(ImGui::Button("Load"))
        {
            resolution->attacks.clear();

            *units = LoadUnits(DATA_PATH + string(unit_filename));
            cout << "Units loaded: " << unit_filename << "\n";

            *level = LoadLevel(level_filename, *units, party);
            cout << "Level loaded: " << level_filename << "\n";

            GoToAIPhase();
        }

        if(ImGui::Button("Save Units"))
        {
            SaveUnits(string(DATA_PATH) + string(unit_filename), *units);
            cout << "Units saved: " << unit_filename << "\n";
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Level"))
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

        ImGui::InputText("units", unit_filename, IM_ARRAYSIZE(unit_filename));
        ImGui::InputText("level", level_filename, IM_ARRAYSIZE(level_filename));
    }
    ImGui::End();

    // Sub-Editors
    UnitEditor(units);
    LevelEditor(level, *units);
    Audio(level);

    // debug
    //ImGui::ShowStyleEditor();
    ImGui::ShowDemoWindow();
}
#endif

#endif
