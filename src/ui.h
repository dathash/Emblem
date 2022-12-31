// Author: Alex Hartford
// Program: Emblem
// File: UI
// Date: December 2022

#ifndef UI_H
#define UI_H

// ======================== General UI helper functions ========================
void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text.c_str());
}


// switch-case to get a ui-printable name from the tile's type enum.
string GetTileNameString(int type)
{
    switch (type)
    {
    case FLOOR: return "Plain";
    case WALL: return "Hill";
    case FOREST: return "Forest";
    case DESERT: return "Swamp";
    case OBJECTIVE: return "Goal";
	default:
		assert(!"ERROR: Unhandled Tile name string in UI.\n");
		return "";
	}
}

// ============================ New UI =========================================
struct UI_State
{
    bool tile_info = false;
    bool unit_blurb = false;
    bool unit_info = false;
    bool combat_preview = false;
    bool game_menu = false;
    bool unit_menu = false;
    bool combat_screen = false;

    void Update()
    {
		// Tile Info
        if(GlobalPlayerTurn
			// TODO: When else don't we want to see this? 
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
				GlobalInterfaceState == SELECTED_OVER_ALLY ||
				GlobalInterfaceState == SELECTED_OVER_ENEMY ||
				GlobalInterfaceState == ATTACK_TARGETING_OVER_TARGET ||
				GlobalInterfaceState == HEALING_TARGETING_OVER_TARGET ||
				GlobalInterfaceState == UNIT_MENU_ROOT
			)
		{
			unit_blurb = true;
		}
		else
		{
			unit_blurb = false;
		}

		// Unit Info
		if(
			GlobalInterfaceState == UNIT_INFO ||
		    GlobalInterfaceState == ENEMY_INFO
			)
		{
			unit_info = true;
		}
		else
		{
			unit_info = false;
		}

		// Combat Preview
		if(GlobalInterfaceState == PREVIEW_ATTACK ||
		   GlobalInterfaceState == PREVIEW_HEALING)
		{
			combat_preview = true;
		}
		else
		{
			combat_preview = false;
		}
    }
};

void DisplayTileInfo(ImGuiWindowFlags wf, const Tile &tile)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(10, 430));
    ImGui::SetNextWindowSize(ImVec2(150, 160));

	//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8);

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Tile", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);

			TextCentered(GetTileNameString(tile.type));
			ImGui::Text("hide %d", tile.avoid);
			ImGui::Text("move %d", tile.penalty);

		ImGui::PopFont();
    }

	//ImGui::PopStyleVar();

    ImGui::End();
}

void DisplayHealthBar(int hp, int maxHp)
{
	float percentHealth = (float)hp / (float)maxHp;
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

// Just display HP bar.
// TODO: Move portrait rendering into this.
void DisplayUnitBlurb(ImGuiWindowFlags wf, const Unit &unit)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(490, 10));
    ImGui::SetNextWindowSize(ImVec2(400, 120));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(unit.name.c_str(), NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);

			DisplayHealthBar(unit.hp, unit.maxHp);

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);
			ImGui::Text("%d / %d", unit.hp, unit.maxHp);
		ImGui::PopFont();
    }
    ImGui::End();
}


// Full display of unit's stats
void DisplayUnitInfo(ImGuiWindowFlags wf, const Unit &unit)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(470, 10));
    ImGui::SetNextWindowSize(ImVec2(420, 180));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Stats", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			ImGui::Text("%s", unit.name.c_str());

			ImGui::SameLine();
			ImGui::Text("[%d/%d]", unit.hp, unit.maxHp);	

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);

			DisplayHealthBar(unit.hp, unit.maxHp);

			// Second line
			ImGui::Text("[ATK %d]", unit.attack);
			ImGui::SameLine();
			ImGui::Text("[MAG %d]", unit.magic);
			ImGui::SameLine();
			ImGui::Text("[DEF %d]", unit.defense);
			ImGui::SameLine();
			ImGui::Text("[RES %d]", unit.resistance);


			ImGui::Text("[HIT %d]", unit.accuracy);
			ImGui::SameLine();
			ImGui::Text("[AVO %d]", unit.avoid);
			ImGui::SameLine();
			ImGui::Text("[CRT %d]", unit.crit);
			ImGui::SameLine();
			ImGui::Text("[Range %d-%d]", unit.minRange, unit.maxRange);
		ImGui::PopFont();
    }
    ImGui::End();
}

// Displays combat preview when initiating combat
void DisplayCombatPreview(ImGuiWindowFlags wf, const Unit &ally, const Unit &enemy)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(470, 10));
    ImGui::SetNextWindowSize(ImVec2(420, 310));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Combat", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			TextCentered(ally.name);

			DisplayHealthBar(ally.hp, ally.maxHp);

			ImGui::BeginTable("Combat", 4, ImGuiTableFlags_RowBg);
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%d", ally.hp);	
				ImGui::TableNextColumn();
				ImGui::Text("%d", CalculateDamage(ally.attack, enemy.defense));
				ImGui::TableNextColumn();
				ImGui::Text("%d", ally.accuracy - enemy.avoid);
				ImGui::TableNextColumn();
				ImGui::Text("%d", ally.crit);
				ImGui::PopStyleColor();

				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("HP");
				ImGui::TableNextColumn();
				ImGui::Text("DMG");
				ImGui::TableNextColumn();
				ImGui::Text("HIT");
				ImGui::TableNextColumn();
				ImGui::Text("CRIT");
				ImGui::PopStyleColor();

				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%d", enemy.hp);
				ImGui::TableNextColumn();
				ImGui::Text("%d", CalculateDamage(enemy.attack, ally.defense));
				ImGui::TableNextColumn();
				ImGui::Text("%d", enemy.accuracy - ally.avoid);
				ImGui::TableNextColumn();
				ImGui::Text("%d", enemy.crit);
				ImGui::PopStyleColor();
			ImGui::EndTable();

			DisplayHealthBar(enemy.hp, enemy.maxHp);

			TextCentered(enemy.name);
		ImGui::PopFont();
    }
    ImGui::End();
}

void RenderUI(UI_State *ui, const Cursor &cursor, const Tilemap &map)
{
    ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoInputs;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(255, 224, 145, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(120, 100, 50, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(120, 100, 50, 255));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5, 0.5));

	if(ui->tile_info)
		DisplayTileInfo(window_flags, map.tiles[cursor.col][cursor.row]);
	if(ui->unit_blurb)
		DisplayUnitBlurb(window_flags, *map.tiles[cursor.col][cursor.row].occupant);
	if(ui->unit_info)
		DisplayUnitInfo(window_flags, *map.tiles[cursor.col][cursor.row].occupant);
	if(ui->combat_preview)
		DisplayCombatPreview(window_flags, *cursor.selected, *cursor.targeted);

	// cleanup
	ImGui::PopStyleVar();

	ImGui::PopStyleColor(3);
}

// ================================= Old UI ====================================
struct Menu
{
    u8 rows;
    u8 current;

    vector<Texture> optionTextTextures;

    Menu(u8 rows_in, u8 current_in, vector<string> options_in)
    : rows(rows_in),
      current(current_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};

#endif
