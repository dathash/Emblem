
#ifndef EDITOR_H
#define EDITOR_H

#if EDITOR
// Renders our imgui stuff.
// Contains static variables that might trip some stuff up, just a heads up.
void EditorPass(vector<unique_ptr<Unit>> *units,
                Level *level)
{
	// ImGui
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    {
        // Internal variables
        static bool showUnitEditor = true;
        static bool showLevelEditor = true;

        static char fileName[128] = INITIAL_UNITS;
        static char levelFileName[128] = INITIAL_LEVEL;

        static uint8_t selectedIndex = 0;

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
        
        if(showLevelEditor)
        {
            ImGui::Begin("level editor");
            {
                ImGui::Text("Hey!\n");
            }
            ImGui::End();
        }

    }
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}
#endif // EDITOR

#endif
