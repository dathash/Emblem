
// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor

#ifndef EDITOR_H
#define EDITOR_H

#if DEV_MODE

static uint8_t selectedIndex = 0;
void UnitEditor(vector<unique_ptr<Unit>> *units)
{
	ImGui::Begin("unit editor");
	{
		if(ImGui::Button("create"))
		{
			units->push_back(make_unique<Unit>(
				string("DEFAULT"),
				SpriteSheet(LoadTextureImage(SPRITES_PATH, string(DEFAULT_SHEET)), 32, ANIMATION_SPEED),
				LoadTextureImage(PORTRAITS_PATH, string(DEFAULT_PORTRAIT)),
				GlobalCurrentID,
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
                NO_BEHAVIOR
			));
			cout << "Created Unit: " << GlobalCurrentID - 1 << "\n";
			++GlobalCurrentID;
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
			ImGui::SameLine();
		}
		ImGui::NewLine();

		assert(selectedIndex < units->size());
		Unit *selected = (*units)[selectedIndex].get();

		ImGui::Text("%s | %d", selected->name.c_str(), selected->id);
        ImGui::InputText("name", &(selected->name));
		// CONSIDER: Update texture sources, spritesheet.
		ImGui::SliderInt("Mov", &selected->mov, 0, 10);
		ImGui::SliderInt("maxHp", &selected->maxHp, 1, 20);
		ImGui::SliderInt("atk", &selected->attack, 0, 20);
		ImGui::SliderInt("abi", &selected->ability, 0, 20);
		ImGui::SliderInt("def", &selected->defense, 0, 20);
		ImGui::SliderInt("acc", &selected->accuracy, 0, 150);
		ImGui::SliderInt("avo", &selected->avoid, 0, 100);
		ImGui::SliderInt("crit", &selected->crit, 0, 100);
		ImGui::SliderInt("min", &selected->minRange, 1, 4);
		ImGui::SliderInt("max", &selected->maxRange, 1, 4);
		ImGui::SliderInt("default ai", (int *)&selected->ai_behavior, 0, 3);
	}
	ImGui::End();
}

// Generate arbitrary debug paths
void 
GenerateDebugPaths(const Level &level, path *path_debug)
{
    static int col = 0;
    static int row = 0;
    static int destCol = 0;
    static int destRow = 0;
    ImGui::Text("From:");
    ImGui::SliderInt("fcol", &col, 0, 10);
    ImGui::SliderInt("frow", &row, 0, 10);
    ImGui::Text("To:");
    ImGui::SliderInt("dcol", &destCol, 0, 10);
    ImGui::SliderInt("drow", &destRow, 0, 10);
    if(ImGui::Button("from"))
    {
        *path_debug = GetPath(level.map, col, row, destCol, destRow, true);
    }
}

void
EditorPollForKeyboardInput(point *editor_cursor)
{
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
    {
        editor_cursor->first = max(editor_cursor->first - 1, 0);
    }
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
    {
        editor_cursor->first = min(editor_cursor->first + 1, 100);
    }
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)))
    {
        editor_cursor->second = max(editor_cursor->second - 1, 0);
    }
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
    {
        editor_cursor->second = min(editor_cursor->second + 1, 100);
    }
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_R)))
    {
        // NOTE: This is temporary, just puts the cursor smack dab in the middle.
        *editor_cursor = point(viewportCol + 7, viewportRow + 5);
    }
}

void LevelEditor(Level *level, const vector<unique_ptr<Unit>> &units)
{
	ImGui::Begin("level editor");
	{
        static point editor_cursor = pair<int, int>(0, 0);
        static path path_debug = {};

        static bool showDebugPaths = false;

        EditorPollForKeyboardInput(&editor_cursor);
        Tile *hover_tile = &level->map.tiles[editor_cursor.first][editor_cursor.second];
        // TODO: This doesn't account for when we're outside the viewport. FIX!!!

        ImGui::Text("Units:");
        if(ImGui::Button("add"))
        {
            if(!(hover_tile->occupant ||
                 hover_tile->type == WALL))
            {
                level->combatants.push_back(make_unique<Unit>(*units[selectedIndex]));
                level->combatants.back()->col = editor_cursor.first;
                level->combatants.back()->row = editor_cursor.second;
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
                                [map_ptr](const unique_ptr<Unit> &u)
                                {
                                    return (map_ptr == u.get());
                                }),
                            level->combatants.end());

                hover_tile->occupant->shouldDie = true;
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
            ImGui::Text("Behavior: %d", level->map.tiles[editor_cursor.first][editor_cursor.second].occupant->ai_behavior);
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
		if(ImGui::Button("FLEE"))
            hover_tile->occupant->ai_behavior = FLEE;

        // =======================  Tile stuff  ================================
        ImGui::Text("Tiles:");
        if(ImGui::Button("none"))
        {
            *hover_tile = FLOOR_TILE;
        }
        ImGui::SameLine();
        if(ImGui::Button("wall"))
        {
            *hover_tile = WALL_TILE;
        }
        ImGui::SameLine();
        if(ImGui::Button("frst"))
        {
            *hover_tile = FOREST_TILE;
        }

        if(ImGui::Button("dsrt"))
        {
            *hover_tile = DESERT_TILE;
        }
        ImGui::SameLine();
        if(ImGui::Button("obj"))
        {
            *hover_tile = OBJECTIVE_TILE;
        }

        ImGui::Checkbox("debug paths", &showDebugPaths);
        if(showDebugPaths)
            GenerateDebugPaths(*level, &path_debug);

        // Render overlays to the main target
        if(path_debug.size() > 0)
        {
            for(pair<int, int> point : path_debug)
            {
                RenderTileColor(point.first - viewportCol,
                           point.second - viewportRow,
                           healColor);
            }
        }

        RenderTileColor(editor_cursor.first - viewportCol,
                   editor_cursor.second - viewportRow,
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
EditorPass(vector<unique_ptr<Unit>> *units,
           Level *level)
{
    // Internal variables
    static bool showUnitEditor = true;
    static bool showLevelEditor = true;
    static bool showGlobals = false;

    static char fileName[128] = INITIAL_UNITS;
    static char levelFileName[128] = INITIAL_LEVEL;

    ImGui::Begin("emblem editor");
    {
        ImGui::Text("Files: <no directory needed>");
        ImGui::InputText("units", fileName, IM_ARRAYSIZE(fileName));
        ImGui::InputText("level", levelFileName, IM_ARRAYSIZE(levelFileName));

        if(ImGui::Button("Load"))
        {
            *units = LoadUnits(string(DATA_PATH) + string(fileName));
            cout << "Units loaded: " << fileName << "\n";
            *level = LoadLevel(string(DATA_PATH) + string(levelFileName), *units);
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

        ImGui::Checkbox("Unit Editor", &showUnitEditor);
        ImGui::Checkbox("Level Editor", &showLevelEditor);
        ImGui::Checkbox("Globals", &showGlobals);

        ImGui::Text("avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    ImGui::End();

    if(showUnitEditor)
        UnitEditor(units);
    if(showLevelEditor)
        LevelEditor(level, *units);
    if(showGlobals)
        GlobalsViewer();

	// debug
	ImGui::ShowStyleEditor();
	ImGui::ShowDemoWindow();
}
#endif // EDITOR

#endif
