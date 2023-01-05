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
    bool options_menu = false;
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
				GlobalInterfaceState == ATTACK_TARGETING ||
				GlobalInterfaceState == HEAL_TARGETING ||
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

		if(GlobalInterfaceState == GAME_MENU_OPTIONS)
		{
			options_menu = true;
		}
		else
		{
			options_menu = false;
		}
    }
};

void DisplayTileInfo(ImGuiWindowFlags wf, const Tile &tile)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(10, 430));
    ImGui::SetNextWindowSize(ImVec2(150, 160));

    ImVec2 lower_left = {10, 430};
    ImVec2 top_left = {10, 10};

    if(quadrant == BOTTOM_LEFT || quadrant == BOTTOM_RIGHT)
        ImGui::SetNextWindowPos(top_left);
    else
        ImGui::SetNextWindowPos(lower_left);
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

// Displays a health bar, with an overlay for the damage to be done.
void DisplayHealthBar(int hp, int maxHp, int damage)
{
	float original_cursor_position = ImGui::GetCursorPosX();
	float original_line_height = ImGui::GetFrameHeight();
	float percentHealth = (((float)hp - damage) / (float)maxHp);
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

// Just display a unit's name and HP bar.
// TODO: Move portrait rendering into this.
void DisplayUnitBlurb(ImGuiWindowFlags wf, const Unit &unit)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(400, 120));

    int x_pos = 490;
    if(!unit.isAlly)
        x_pos = 170;

    ImVec2 top_right = ImVec2(x_pos, 10);
    ImVec2 bottom_right = ImVec2(x_pos, 470);

    if(quadrant == TOP_LEFT || quadrant == TOP_RIGHT)
        ImGui::SetNextWindowPos(bottom_right);
    else
        ImGui::SetNextWindowPos(top_right);

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(unit.name.c_str(), NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);

			DisplayHealthBar(unit.hp, unit.maxHp, 0);

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
    ImGui::SetNextWindowSize(ImVec2(480, 180));

    int x_pos = 410;
    if(!unit.isAlly)
        x_pos = 170;

    ImVec2 top_right = ImVec2(x_pos, 10);
    ImVec2 bottom_right = ImVec2(x_pos, 410);

    if(quadrant == TOP_LEFT || quadrant == TOP_RIGHT)
        ImGui::SetNextWindowPos(bottom_right);
    else
        ImGui::SetNextWindowPos(top_right);

    // Logic
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

			DisplayHealthBar(unit.hp, unit.maxHp, 0);

			// Second line
			ImGui::Text("[ATK %d]", unit.attack);
			ImGui::SameLine();
			ImGui::Text("[DEF %d]", unit.defense);
			ImGui::SameLine();
			ImGui::Text("[MAG %d]", unit.magic);
			ImGui::SameLine();
			ImGui::Text("[RES %d]", unit.resistance);
			ImGui::SameLine();
			ImGui::Text("[HEAL %d]", unit.healing);

			ImGui::Text("[HIT %d%%]", unit.accuracy);
			ImGui::SameLine();
			ImGui::Text("[AVO %d%%]", unit.avoid);
			ImGui::SameLine();
			ImGui::Text("[CRT %d%%]", unit.crit);
			ImGui::SameLine();
			ImGui::Text("[RG %d-%d]", unit.minRange, unit.maxRange);
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

// Returns the color corresponding to the likelihood of a hit.
// Blue for high, red for low.
ImU32
GetHitColor(int hit)
{
    if(hit > 70) // arbitrary TODO: check if there's a good way to make this feel intuitive.
        return IM_COL32(0, 0, 255, 255);
    return IM_COL32(255, 0, 0, 255);
}

// Displays combat preview when initiating combat
void
DisplayCombatPreview(ImGuiWindowFlags wf, const Unit &ally, const Unit &enemy)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(460, 220));

    ImVec2 top_right = {170, 10};
    ImVec2 bottom_right = {170, 370};

    if(quadrant == TOP_LEFT || quadrant == TOP_RIGHT)
        ImGui::SetNextWindowPos(bottom_right);
    else
        ImGui::SetNextWindowPos(top_right);

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Combat", NULL, wf);
    {
        Outcome outcome = PredictCombat(ally, enemy, 
                          ManhattanDistance(point(ally.col, ally.row), 
                                            point(enemy.col, enemy.row)));

		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("%s", ally.name.c_str());

			ImGui::BeginTable("Combat", 3, ImGuiTableFlags_RowBg);
				ImGui::TableNextColumn();

		        ImGui::PushFont(uiFontLarge);
				ImGui::PushStyleColor(ImGuiCol_Text, GetHealthColor(ally.hp, ally.maxHp));
				ImGui::Text("%d", ally.hp);
				ImGui::PopStyleColor();
                ImGui::SameLine();
				ImGui::Text("/");
                ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, GetHealthColor(outcome.one_health, ally.maxHp));
				ImGui::Text("%d", outcome.one_health);
				ImGui::PopStyleColor();
				ImGui::TableNextColumn();
		        ImGui::PopFont();

				ImGui::PushStyleColor(ImGuiCol_Text, GetHitColor(outcome.one_hit));
				ImGui::Text("%d%%", outcome.one_hit);
				ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("hit");

				ImGui::TableNextColumn();
				ImGui::Text("%d%% crit", outcome.one_crit);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

		        ImGui::PushFont(uiFontLarge);
				ImGui::PushStyleColor(ImGuiCol_Text, GetHealthColor(enemy.hp, enemy.maxHp));
				ImGui::Text("%d", enemy.hp);
				ImGui::PopStyleColor();
                ImGui::SameLine();
				ImGui::Text("/");
                ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, GetHealthColor(outcome.two_health, enemy.maxHp));
				ImGui::Text("%d", outcome.two_health);
				ImGui::PopStyleColor();
		        ImGui::PopFont();

				ImGui::TableNextColumn();
                if(outcome.two_hit)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, GetHitColor(outcome.two_hit));
                    ImGui::Text("%d%%", outcome.two_hit);
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("hit");

                    ImGui::TableNextColumn();
                    ImGui::Text("%d%% crit", outcome.two_crit);
                }
                else  // don't display values if the enemy won't do anything.
                {
                    ImGui::Text("- hit");
                    ImGui::TableNextColumn();
                    ImGui::Text("- crit");
                }

			ImGui::EndTable();

            ImGui::Text("%s", enemy.name.c_str());
		ImGui::PopFont();
    }
    ImGui::End();
}

// Displays options menu
void DisplayOptionsMenu(ImGuiWindowFlags wf)
{
	static bool bl;
	ImGui::SetNextWindowFocus();
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(400, 300), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(420, 310));

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Options", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			ImGui::Checkbox("checkbox1", &bl);
			ImGui::Checkbox("checkbox2", &bl);
			ImGui::Checkbox("checkbox3", &bl);
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
	if(ui->options_menu)
		DisplayOptionsMenu(window_flags);

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
    vector<string> optionText;

    Menu(u8 rows_in, u8 current_in, vector<string> options_in)
    : rows(rows_in),
      current(current_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), uiTextColor));
            optionText.push_back(s);
        }
    }

    // Custom-build a menu based on your current options.
    // Mostly used to dynamically display attack/heal/trade options.
    void
    AddOption(string s)
    {
        rows += 1;
        optionTextTextures.push_back(LoadTextureText(s.c_str(), uiTextColor));
        optionText.push_back(s);
    }
};

#endif
