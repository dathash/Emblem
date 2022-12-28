
// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Editor
// Date: December 2022

#ifndef EDITOR_H
#define EDITOR_H

#if EDITOR

void UnitEditor(vector<unique_ptr<Unit>> *units)
{
	static uint8_t selectedIndex = 0;

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
				3
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
		// TODO
		// Update name
		// Update texture sources, spritesheet.
		ImGui::SliderInt("Mov", &selected->mov, 0, 10);
		ImGui::SliderInt("maxHp", &selected->maxHp, 1, 20);
		ImGui::SliderInt("min", &selected->minRange, 1, 3);
		ImGui::SliderInt("max", &selected->maxRange, 1, 3);
		ImGui::SliderInt("atk", &selected->attack, 0, 10);
		ImGui::SliderInt("def", &selected->defense, 0, 10);
		ImGui::SliderInt("acc", &selected->accuracy, 0, 100);
	}
	ImGui::End();
}

void LevelEditor(Level *level)
{
	ImGui::Begin("level editor");
	{
        //TODO: point and click feature plz!!!
        ImGui::Text("Cursor:");
        ImGui::SliderInt("col", &point_debug.first, 0, 10);
        ImGui::SliderInt("row", &point_debug.second, 0, 10);
        static int col = 0;
        static int row = 0;
        static int destCol = 0;
        static int destRow = 0;
        ImGui::Text("From:");
        ImGui::SliderInt("col", &col, 0, 10);
        ImGui::SliderInt("row", &row, 0, 10);
        ImGui::Text("To:");
        ImGui::SliderInt("dcol", &destCol, 0, 10);
        ImGui::SliderInt("drow", &destRow, 0, 10);
        if(ImGui::Button("from"))
        {
            path_debug = GetPath(level->map, col, row, destCol, destRow);
        }
        if(ImGui::Button("distance"))
        {
            PrintDistanceField(DistanceField(level->map, 0, 0));
        }
	}
	ImGui::End();
}

// Renders all imgui stuff.
// Contains static variables that might trip some stuff up, just a heads up.
void EditorPass(vector<unique_ptr<Unit>> *units,
                Level *level)
{
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    {
        // Internal variables
        static bool showUnitEditor = true;
        static bool showLevelEditor = true;

        static char fileName[128] = INITIAL_UNITS;
        static char levelFileName[128] = INITIAL_LEVEL;

        ImGui::Begin("emblem editor");
        {
            ImGui::Text("Files: <no directory needed>");
            ImGui::InputText("units", fileName, IM_ARRAYSIZE(fileName));
            ImGui::InputText("level", levelFileName, IM_ARRAYSIZE(levelFileName));

            if(ImGui::Button("Load"))
            {
                *units = LoadCharacters(string(DATA_PATH) + string(fileName));
                cout << "Characters loaded: " << fileName << "\n";
                *level = LoadLevel(string(DATA_PATH) + string(levelFileName), *units);
                cout << "Level reloaded: " << levelFileName << "\n";
            }
            ImGui::SameLine();
            if(ImGui::Button("Save Ch"))
            {
                SaveCharacters(string(DATA_PATH) + string(fileName), *units);
                cout << "Characters saved: " << fileName << "\n";
            }
            ImGui::SameLine();
            if(ImGui::Button("Save Level"))
            {
                SaveLevel(string(DATA_PATH) + string(levelFileName), *level);
                cout << "Level saved: " << levelFileName << "\n";
            }

            ImGui::Checkbox("Unit Editor", &showUnitEditor);
            ImGui::Checkbox("Level Editor", &showLevelEditor);

            ImGui::Text("avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();

        if(showUnitEditor)
        {
			UnitEditor(units);
        }
        
        if(showLevelEditor)
        {
			LevelEditor(level);
        }
    }
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}
#endif // EDITOR

#endif
