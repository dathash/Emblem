
// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor

#ifndef EDITOR_H
#define EDITOR_H

#if DEV_MODE

void
Meta()
{
	ImGui::Begin("Meta");
	{
        static float music_volume = DEFAULT_MUSIC_VOLUME;
        static float sfx_volume = DEFAULT_SFX_VOLUME;
        if(ImGui::SliderFloat("Music", &music_volume, 0.0f, 1.0f))
            SetMusicVolume(music_volume);
        if(ImGui::SliderFloat("SFX", &sfx_volume, 0.0f, 1.0f))
            SetSfxVolume(sfx_volume);
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
				Spritesheet(LoadTextureImage(SPRITES_PATH, string(DEFAULT_SHEET)), 32, ANIMATION_SPEED),
				LoadTextureImage(FULLS_PATH, string(DEFAULT_PORTRAIT)),
				true,
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
				3,
				ABILITY_NONE,
                NO_BEHAVIOR
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

		assert(selectedIndex < units->size());
		Unit *selected = (*units)[selectedIndex].get();

		ImGui::Text("%s | %zu", selected->name.c_str(), selected->ID());
        ImGui::InputText("name", &(selected->name));
		ImGui::SliderInt("mov", &selected->movement, 0, 10);
		ImGui::SliderInt("hp", &selected->max_health, 1, 50);
		ImGui::SliderInt("atk", &selected->attack, 0, 20);
		ImGui::SliderInt("def", &selected->defense, 0, 20);
		ImGui::SliderInt("apt", &selected->aptitude, 0, 20);
		ImGui::SliderInt("spd", &selected->speed, 0, 20);
		ImGui::SliderInt("acc", &selected->accuracy, 0, 150);
		ImGui::SliderInt("avo", &selected->avoid, 0, 100);
		ImGui::SliderInt("crit", &selected->crit, 0, 100);
		ImGui::SliderInt("min", &selected->min_range, 1, 4);
		ImGui::SliderInt("max", &selected->max_range, 1, 4);
		ImGui::SliderInt("default ai", (int *)&selected->ai_behavior, 0, 3);
        ImGui::Text("ability");
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

        static bool showDebugPaths = false;

        EditorPollForKeyboardInput(&editor_cursor, level->map.width, level->map.height);
        Tile *hover_tile = &level->map.tiles[editor_cursor.col][editor_cursor.row];

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
            if(ImGui::Button("Dmg"))
            {
                hover_tile->occupant->Damage(1);
            }
            ImGui::SameLine();
            if(ImGui::Button("Heal"))
            {
                hover_tile->occupant->Damage(-1);
            }

            ImGui::Text("AI Behavior");
            if(ImGui::Button("NONE"))
                hover_tile->occupant->ai_behavior = NO_BEHAVIOR;
            ImGui::SameLine();
            if(ImGui::Button("PURSUE"))
                hover_tile->occupant->ai_behavior = PURSUE;
            ImGui::SameLine();
            if(ImGui::Button("BOLSTER"))
                hover_tile->occupant->ai_behavior = BOLSTER;
            ImGui::SameLine();
            if(ImGui::Button("wta"))
                hover_tile->occupant->ai_behavior = WAIT_THEN_ATTACK;
        }

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

        RenderTileColor({editor_cursor.col - viewportCol,
                   editor_cursor.row - viewportRow},
                   editorColor);
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
            *level = LoadLevel(DATA_PATH + string(levelFileName), *units);
            ReloadLevel();
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
            *level = LoadLevel(DATA_PATH + string("test.txt"), *units);
            sprintf(levelFileName, "test.txt");
            ReloadLevel();
        }

        int wrap = 0;
        for(const string &s : levels)
        {
            if(ImGui::Button(s.c_str()))
            {
                *level = LoadLevel(DATA_PATH + s, *units);
                sprintf(levelFileName, "%s", s.c_str());
                ReloadLevel();
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
        Meta();

    /*
	// debug
	ImGui::ShowStyleEditor();
	ImGui::ShowDemoWindow();
    */
}
#endif // EDITOR

#endif
