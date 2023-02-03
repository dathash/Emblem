

// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor

#ifndef EDITOR_H
#define EDITOR_H

#if DEV_MODE

void
Meta(Level *level)
{
    ImGui::Begin("Meta");
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
static bool editing_allies = false;
void
UnitEditor(vector<shared_ptr<Unit>> *units)
{
    ImGui::Begin("unit editor");
    {
        ImGui::Checkbox("allies", &editing_allies);
        if(ImGui::Button("create"))
        {
            units->push_back(make_shared<Unit>(
                string("DEFAULT_CHANGE"),
                false,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                ABILITY_NONE,
                NO_BEHAVIOR,
                3,

                3,
                3,
                3,
                3,
                3,
                3,

                Spritesheet(LoadTextureImage(SPRITES_PATH, string(DEFAULT_SHEET)), 32, ANIMATION_SPEED),
                LoadTextureImage(FULLS_PATH, string(DEFAULT_PORTRAIT)),
                LoadTextureImage(FULLS_PATH, string(DEFAULT_PORTRAIT)),
                LoadTextureImage(FULLS_PATH, string(DEFAULT_PORTRAIT)),
                LoadTextureImage(FULLS_PATH, string(DEFAULT_PORTRAIT))
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
            if((*units)[i]->is_ally == editing_allies)
            {
                char buffer[256];
                sprintf(buffer, "%d", i);
                if (ImGui::Button(buffer))
                    selectedIndex = i;
                if(i % 7 || i == 0)
                    ImGui::SameLine();
            }
        }
        ImGui::NewLine();

        SDL_assert(selectedIndex < units->size());
        Unit *selected = (*units)[selectedIndex].get();

        ImGui::Text("%s | %zu", selected->name.c_str(), selected->ID());
        ImGui::InputText("name", &(selected->name));
        ImGui::SliderInt("mov", &selected->movement, 0, 10);
        ImGui::SliderInt("hp", &selected->max_health, 1, 50);
        ImGui::SliderInt("atk", &selected->attack, 0, 20);
        ImGui::SliderInt("def", &selected->defense, 0, 20);
        ImGui::SliderInt("apt", &selected->aptitude, 0, 20);
        ImGui::SliderInt("spd", &selected->speed, 0, 20);
        ImGui::SliderInt("skl", &selected->skill, 0, 20);
        ImGui::SliderInt("min", &selected->min_range, 1, 4);
        ImGui::SliderInt("max", &selected->max_range, 1, 4);
        ImGui::SliderInt("level", &selected->level, 1, 20);
        ImGui::SliderInt("default ai", (int *)&selected->ai_behavior, 0, 5);

        ImGui::Text("growths");
        ImGui::SliderInt("health", (int *)&selected->growths.health, 0, 100);
        ImGui::SliderInt("attack", (int *)&selected->growths.attack, 0, 100);
        ImGui::SliderInt("aptitude", (int *)&selected->growths.aptitude, 0, 100);
        ImGui::SliderInt("defense", (int *)&selected->growths.defense, 0, 100);
        ImGui::SliderInt("speed", (int *)&selected->growths.speed, 0, 100);
        ImGui::SliderInt("skill", (int *)&selected->growths.skill, 0, 100);


        ImGui::Text("ability: %d", selected->ability);
        ImGui::SameLine();
        if(ImGui::Button("N"))
            selected->ability = ABILITY_NONE;
        ImGui::SameLine();
        if(ImGui::Button("H"))
            selected->ability = ABILITY_HEAL;
        ImGui::SameLine();
        if(ImGui::Button("B"))
            selected->ability = ABILITY_BUFF;
        ImGui::SameLine();
        if(ImGui::Button("S"))
            selected->ability = ABILITY_SHIELD;
        ImGui::SameLine();
        if(ImGui::Button("D"))
            selected->ability = ABILITY_DANCE;
        ImGui::SliderInt("xpv", &selected->xp_value, 0, 200);
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
    if(!WithinViewport(desired_move))
    {
        MoveViewport(desired_move);
    }
    *editor_cursor = desired_move;
}

void LevelEditor(Level *level, const vector<shared_ptr<Unit>> &units)
{
    ImGui::Begin("level editor");
    {
        static position editor_cursor = {0, 0};
        static path path_debug = {};

        EditorPollForKeyboardInput(&editor_cursor, level->map.width, level->map.height);
        Tile *hover_tile = &level->map.tiles[editor_cursor.col][editor_cursor.row];

        ImGui::Text("Objective: %s", GetObjectiveString(level->objective).c_str());
        if(ImGui::Button("rout"))
        {
            level->objective = OBJECTIVE_ROUT;
        }
        ImGui::SameLine();
        if(ImGui::Button("capture"))
        {
            level->objective = OBJECTIVE_CAPTURE;
        }
        ImGui::SameLine();
        if(ImGui::Button("boss"))
        {
            level->objective = OBJECTIVE_BOSS;
        }

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
        ImGui::SameLine();
        if(ImGui::Button("spawn"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = SPAWN_TILE;
            hover_tile->occupant = tmp;
        }
        ImGui::SameLine();
        if(ImGui::Button("fort"))
        {
            Unit *tmp = hover_tile->occupant;
            *hover_tile = FORT_TILE;
            hover_tile->occupant = tmp;
        }

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
            ImGui::SameLine();
            ImGui::Text("Behavior: %d", hover_tile->occupant->ai_behavior);
            ImGui::SameLine();
            ImGui::Checkbox("boss?", &hover_tile->occupant->is_boss);

            if(ImGui::Button("Dmg"))
            {
                hover_tile->occupant->Damage(1);
            }
            ImGui::SameLine();
            if(ImGui::Button("Heal"))
            {
                hover_tile->occupant->Damage(-1);
            }
            ImGui::SameLine();
            if(ImGui::Button("lvl+"))
            {
                hover_tile->occupant->GrantExperience(100);
            }
            ImGui::SameLine();
            if(ImGui::Button("25xp"))
            {
                hover_tile->occupant->GrantExperience(25);
            }

            ImGui::Text("AI Behavior");
            if(ImGui::Button("-"))
                hover_tile->occupant->ai_behavior = NO_BEHAVIOR;
            ImGui::SameLine();
            if(ImGui::Button("P"))
                hover_tile->occupant->ai_behavior = PURSUE;
            ImGui::SameLine();
            if(ImGui::Button("Pa1"))
                hover_tile->occupant->ai_behavior = PURSUE_AFTER_1;
            ImGui::SameLine();
            if(ImGui::Button("Pa2"))
                hover_tile->occupant->ai_behavior = PURSUE_AFTER_2;
            ImGui::SameLine();
            if(ImGui::Button("Pa3"))
                hover_tile->occupant->ai_behavior = PURSUE_AFTER_3;

            if(ImGui::Button("BOSS"))
                hover_tile->occupant->ai_behavior = BOSS;
            ImGui::SameLine();
            if(ImGui::Button("BtM"))
                hover_tile->occupant->ai_behavior = BOSS_THEN_MOVE;
            ImGui::SameLine();
            if(ImGui::Button("Range"))
                hover_tile->occupant->ai_behavior = ATTACK_IN_RANGE;
            ImGui::SameLine();
            if(ImGui::Button("AI2"))
                hover_tile->occupant->ai_behavior = ATTACK_IN_TWO;

            if(ImGui::Button("Flee"))
                hover_tile->occupant->ai_behavior = FLEE;
            ImGui::SameLine();
            if(ImGui::Button("Plunder"))
                hover_tile->occupant->ai_behavior = TREASURE_THEN_FLEE;
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
                RenderTileColor({p.col - viewportCol,
                                 p.row - viewportRow},
                           healColor);
            }
        }
        */

        if(WithinViewport(editor_cursor))
        {
            RenderTileColor({editor_cursor.col - viewportCol,
                       editor_cursor.row - viewportRow},
                       editorColor);
        }
    }
    ImGui::End();
}

void
GlobalsViewer()
{
    ImGui::Begin("Globals");
    {
        ImGui::Text("State");
        ImGui::Checkbox("GlobalRunning", &GlobalRunning);
        ImGui::Checkbox("GlobalPlayerTurn", &GlobalPlayerTurn);
        ImGui::Checkbox("GlobalEditorMode", &GlobalEditorMode);
        ImGui::Text("%02d | STATE", GlobalInterfaceState);
        ImGui::Text("%02d | AI", GlobalAIState);
    }
    ImGui::End();
}

// Renders all imgui stuff.
// Contains static variables that might trip some stuff up, just a heads up.
void
EditorPass(vector<shared_ptr<Unit>> *units,
           const vector<shared_ptr<Unit>> &party,
           Level *level, const vector<string> &levels)
{
    // Internal variables
    static bool showUnitEditor = true;
    static bool showLevelEditor = true;
    static bool showGlobals = false;
    static bool showMeta = true;

    static char fileName[128] = INITIAL_UNITS;
    static char levelFileName[128] = INITIAL_LEVEL;

    ImGui::Begin("emblem editor");
    {
        if(ImGui::Button("Load"))
        {
            *units = LoadUnits(DATA_PATH + string(fileName));
            cout << "Units loaded: " << fileName << "\n";
            *level = LoadLevel(DATA_PATH + string(levelFileName), *units, party);
            GlobalAIState = PLAYER_TURN;
            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
            cout << "Level loaded: " << levelFileName << "\n";
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Units"))
        {
            SaveUnits(string(DATA_PATH) + string(fileName), *units);
            cout << "Units saved: " << fileName << "\n";
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Level"))
        {
            SaveLevel(string(DATA_PATH) + string(levelFileName), *level);
            cout << "Level saved: " << levelFileName << "\n";
        }

        if(ImGui::Button("Test Zone"))
        {
            *level = LoadLevel(DATA_PATH + string("test.txt"), *units, party);
            sprintf(levelFileName, "test.txt");
            GlobalAIState = PLAYER_TURN;
            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
        }

        int wrap = 0;
        for(const string &s : levels)
        {
            if(ImGui::Button(s.c_str()))
            {
                *level = LoadLevel(DATA_PATH + s, *units, party);
                sprintf(levelFileName, "%s", s.c_str());
                GlobalAIState = PLAYER_TURN;
                GlobalPlayerTurn = true;
                GlobalTurnStart = true;
            }
            if(++wrap % 4)
                ImGui::SameLine();
        }

        ImGui::InputText("units", fileName, IM_ARRAYSIZE(fileName));
        ImGui::InputText("level", levelFileName, IM_ARRAYSIZE(levelFileName));

        ImGui::Checkbox("Unit Editor", &showUnitEditor);
        ImGui::Checkbox("Level Editor", &showLevelEditor);
        ImGui::Checkbox("Globals", &showGlobals);
        ImGui::Checkbox("Meta", &showMeta);

        ImGui::Text("avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    ImGui::End();

    if(showUnitEditor)
        UnitEditor(units);
    if(showLevelEditor)
        LevelEditor(level, *units);
    if(showGlobals)
        GlobalsViewer();
    if(showMeta)
        Meta(level);

    /*
    // debug
    ImGui::ShowStyleEditor();
    ImGui::ShowDemoWindow();
    */
}
#endif // EDITOR

#endif
