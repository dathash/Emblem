// Author: Alex Hartford
// Program: Emblem
// File: UI

#ifndef UI_H
#define UI_H

// ======================== General UI helper functions ========================
void 
TextCentered(string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text.c_str());
}


// switch-case to get a ui-printable name from the tile's type enum.
string 
GetTileNameString(TileType type)
{
    switch (type)
    {
    case FLOOR: return "Plain";
    case WALL: return "Hill";
    case FOREST: return "Forest";
    case SWAMP: return "Swamp";
    case GOAL: return "Goal";
    case FORT: return "Fort";
    case VILLAGE: return "House";
    case CHEST: return "Chest";
	default:
		assert(!"ERROR: Unhandled Tile name string in UI.\n");
		return "";
	}
}

string
GetDieString(int (*die)())
{
    if(die == d1)
        return "d1";
    else if(die == d4)
        return "d4";
    else if(die == d6)
        return "d6";
    else if(die == d8)
        return "d8";
    else if(die == d10)
        return "d10";
    else if(die == d12)
        return "d12";
    cout << "WARNING: Bad die format: " << die << "\n";
    return "";
}

int
GetMaxValue(int (*die)())
{
    if(die == d1)
        return 1;
    else if(die == d4)
        return 4;
    else if(die == d6)
        return 6;
    else if(die == d8)
        return 8;
    else if(die == d10)
        return 10;
    else if(die == d12)
        return 12;
    cout << "WARNING: Bad die format: " << die << "\n";
    return 0;
}

// ============================ New UI =========================================
struct UI_State
{
    bool tile_info = false;
    bool unit_blurb = false;
    bool unit_selected = false;
    bool attack_targeting = false;
    bool game_menu = false;
    bool game_over = false;
    bool unit_menu = false;

    void
    Clear()
    {
        tile_info = false;
        unit_blurb = false;
        unit_selected = false;
        attack_targeting = false;
        game_menu = false;
        game_over = false;
        unit_menu = false;
    }

    void 
    Update()
    {
		// Tile Info
        if(
            GlobalInterfaceState == NEUTRAL_OVER_GROUND || 
            GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
            GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
            GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
            GlobalInterfaceState == SELECTED ||
            GlobalInterfaceState == ATTACK_THINKING ||
            GlobalInterfaceState == ATTACK_TARGETING
			)
        {
            tile_info = true;
        }
        else
        {
            tile_info = false;
        }

		// Unit Blurb
		if(
            GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
            GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
            GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
            GlobalInterfaceState == ATTACK_THINKING
            )
		{
			unit_blurb = true;
		}
		else
		{
			unit_blurb = false;
		}

        if(GlobalInterfaceState == SELECTED)
        {
            unit_selected = true;
        }
        else
        {
            unit_selected = false;
        }
        if(GlobalInterfaceState == ATTACK_TARGETING)
        {
            attack_targeting = true;
        }
        else
        {
            attack_targeting = false;
        }

		if(GlobalInterfaceState == GAME_OVER)
		{
			game_over = true;
		}
		else
		{
			game_over = false;
		}
    }
};

void
DisplayTitleScreen(ImGuiWindowFlags wf)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(200, 100));
    ImGui::SetNextWindowSize(ImVec2(800, 550));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Emblem", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			ImGui::Text("Welcome to Tyrian.");
			ImGui::Text("The year is 399, CE.");
			ImGui::TextWrapped("The empires of Imrryr and Tolmec struggle against the plagues of the middle dark ages.");
			ImGui::TextWrapped("The hidden nomads of Gaul sit in gentle repose, watching the world from the spires of Erenor.");
			ImGui::TextWrapped("And beneath the cerulean skies, the gears of fate are set into motion.");
			ImGui::NewLine();
			TextCentered("-WASD- to move.");
			TextCentered("-[SPACE]- to select");
			TextCentered("-[SHIFT]- to deselect");
			TextCentered("-E- to display information");
		ImGui::PopFont();
    }
    ImGui::End();

    return;
}

void 
DisplayTileInfo(ImGuiWindowFlags wf, const Tile &tile)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(450, 500));
    ImGui::SetNextWindowSize(ImVec2(150, 40));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(GetTileNameString(tile.type).c_str(), NULL, wf);
    {
		ImGui::PushFont(uiFontMedium);
		ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopFont();
}

// Displays a health bar, with an overlay for the damage to be done.
void 
DisplayHealthBar(int health, int max_health, int damage)
{
	float percentHealth = (((float)health - damage) / (float)max_health);
	percentHealth = (0 < percentHealth ? percentHealth : 0);
	if(percentHealth > 0.66)
	{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 0, 255));
	}
	else if(percentHealth > 0.33)
	{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 255, 0, 255));
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 255));
	}
	ImGui::ProgressBar(percentHealth, ImVec2(-1.0f, 0.0f));
	ImGui::PopStyleColor();
}

void
DisplayEquipInfo(const Equip *equip)
{
    ImGui::Text("[%s]", equip->name.c_str());
}

void 
DisplayUnitBlurb(ImGuiWindowFlags wf, const Unit &unit)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(250, 180));
    ImGui::SetNextWindowPos(ImVec2(625, 500));

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(unit.name.c_str(), NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);

			DisplayHealthBar(unit.health, unit.max_health, 0);

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);
            ImGui::Text("%d MOV", unit.movement);
            ImGui::SameLine(ImGui::GetWindowWidth() - 80);
			ImGui::Text("%d/%d HP", unit.health, unit.max_health);

            if(unit.primary)
            {
                DisplayEquipInfo(unit.primary);
            }

            if(unit.secondary)
            {
                ImGui::SameLine();
                DisplayEquipInfo(unit.secondary);
            }
		ImGui::PopFont();
    }
    ImGui::End();
}

// Returns the color corresponding to the health of a unit.
// Blue for high, red for low.
ImU32
GetHealthColor(int val, int max)
{
    if(val * 2 > max)
        return IM_COL32(0, 0, 255, 255);
    return IM_COL32(255, 0, 0, 255);
}

void 
DisplayGameOver(ImGuiWindowFlags wf)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(400, 300), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(420, 310));

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Game Over", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("You lost.");
            ImGui::Text("[SPACE] to retry");
            ImGui::Text("[SHIFT] to quit");
		ImGui::PopFont();
    }
    ImGui::End();
}

void 
RenderUI(UI_State *ui, 
         const Cursor &cursor, 
         const Level &level
        )
{
    ui->Update();

    ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoInputs;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, SdlToImColor(uiColor));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, SdlToImColor(uiTitleColor));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, SdlToImColor(uiTitleColor));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5, 0.5));

    // TITLE SCREEN
    if(GlobalInterfaceState == TITLE_SCREEN)
    {
        DisplayTitleScreen(window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return;
    }

    if(!GlobalPlayerTurn)
    {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return;
    }

    // USER MODE UI
	if(ui->tile_info)
		DisplayTileInfo(window_flags, level.map.tiles[cursor.pos.col][cursor.pos.row]);
	if(ui->unit_blurb)
		DisplayUnitBlurb(window_flags, *level.map.tiles[cursor.pos.col][cursor.pos.row].occupant);
	if(ui->unit_selected)
		DisplayUnitBlurb(window_flags, *cursor.selected);
	if(ui->attack_targeting)
		DisplayUnitBlurb(window_flags, *cursor.selected);
    if(ui->game_over)
        DisplayGameOver(window_flags);

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
}

#endif
