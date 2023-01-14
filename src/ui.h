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
GetTileNameString(int type)
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
    bool game_over = false;
    bool unit_menu = false;
    bool combat_screen = false;
    bool timer = false;

    void
    Clear()
    {
        tile_info = false;
        unit_blurb = false;
        unit_info = false;
        combat_preview = false;
        game_menu = false;
        game_over = false;
        unit_menu = false;
        combat_screen = false;
        timer = false;
    }

    void 
    Update()
    {
		// Tile Info
        if(
           !(GlobalInterfaceState == LEVEL_MENU ||
             GlobalInterfaceState == CONVERSATION ||
             GlobalInterfaceState == PREVIEW_ATTACK ||
             GlobalInterfaceState == PREVIEW_HEALING)
			)
        {
            tile_info = true;
        }
        else
        {
            tile_info = false;
        }

        // Timer
        if(
           !(GlobalInterfaceState == LEVEL_MENU ||
             GlobalInterfaceState == CONVERSATION)
          )
        {
            timer = true;
        }
        else
        {
            timer = false;
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
DisplayTileInfo(ImGuiWindowFlags wf, const Tile &tile, enum quadrant quad)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(10, 430));
    ImGui::SetNextWindowSize(ImVec2(150, 160));

    ImVec2 lower_left = {10, 430};
    ImVec2 top_left = {10, 10};

    if(quad == BOTTOM_LEFT || quad == BOTTOM_RIGHT)
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
void 
DisplayHealthBar(int health, int max_health, int damage)
{
	float original_cursor_position = ImGui::GetCursorPosX();
	float original_line_height = ImGui::GetFrameHeight();
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

// Just display a unit's name and health bar.
// CONSIDER: Move portrait rendering into this.
void 
DisplayUnitBlurb(ImGuiWindowFlags wf, const Unit &unit, enum quadrant quad)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(400, 120));

    int x_pos = 490;
    if(!unit.is_ally)
        x_pos = 170;

    ImVec2 top_right = ImVec2(x_pos, 10);
    ImVec2 bottom_right = ImVec2(x_pos, 470);

    if(quad == TOP_LEFT || quad == TOP_RIGHT)
        ImGui::SetNextWindowPos(bottom_right);
    else
        ImGui::SetNextWindowPos(top_right);

    // Logic
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(unit.name.c_str(), NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);

			DisplayHealthBar(unit.health, unit.max_health, 0);

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);
			ImGui::Text("%d / %d", unit.health, unit.max_health);
		ImGui::PopFont();
    }
    ImGui::End();
}

// Full display of unit's stats
void 
DisplayUnitInfo(ImGuiWindowFlags wf, const Unit &unit, enum quadrant quad)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(480, 180));

    int x_pos = 410;
    if(!unit.is_ally)
        x_pos = 170;

    ImVec2 top_right = ImVec2(x_pos, 10);
    ImVec2 bottom_right = ImVec2(x_pos, 410);

    if(quad == TOP_LEFT || quad == TOP_RIGHT)
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
			ImGui::Text("[%d/%d]", unit.health, unit.max_health);

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);

			DisplayHealthBar(unit.health, unit.max_health, 0);

			// Second line
            ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkRed));
			ImGui::Text("[ATK %d]", unit.attack);
            ImGui::PopStyleColor();
			ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkBlue));
			ImGui::Text("[DEF %d]", unit.defense);
            ImGui::PopStyleColor();
			ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkGreen));
			ImGui::Text("[ABI %d]", unit.ability);
            ImGui::PopStyleColor();

			ImGui::Text("[HIT %d%%]", unit.accuracy);
			ImGui::SameLine();
			ImGui::Text("[AVO %d%%]", unit.avoid);
			ImGui::SameLine();
			ImGui::Text("[CRT %d%%]", unit.crit);
			ImGui::SameLine();
			ImGui::Text("[RG %d-%d]", unit.min_range, unit.max_range);
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
    if(hit > 70) // arbitrary | CONSIDER: check if there's a good way to make this feel intuitive.
        return IM_COL32(0, 0, 255, 255);
    return IM_COL32(255, 0, 0, 255);
}

// Displays combat preview when initiating combat
void
DisplayCombatPreview(ImGuiWindowFlags wf, const Unit &ally, const Unit &target,
                                          int ally_avoid_bonus, int enemy_avoid_bonus)
{
    // Predict the outcome
    Outcome outcome;
    if(target.is_ally)
    {
        outcome = PredictHealing(ally, target);
    }
    else
    {
        outcome = PredictCombat(ally, target,
                  ManhattanDistance(ally.pos, target.pos),
                  ally_avoid_bonus, enemy_avoid_bonus);
    }

	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(200, 200));
    ImGui::SetNextWindowPos(ImVec2(50, 400));

    // Render
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(ally.name.c_str(), NULL, wf);
    {

		ImGui::PushFont(uiFontMedium);
            ImGui::Text("%d atk", outcome.one_attack);
            ImGui::Text("%d%% hit", outcome.one_hit);
            ImGui::Text("%d%% crit", outcome.one_crit);
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(200, 200));
    ImGui::SetNextWindowPos(ImVec2(620, 400));

    ImGui::Begin(target.name.c_str(), NULL, wf);
    {
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("%d atk", outcome.two_attack);
            ImGui::Text("%d%% hit", outcome.two_hit);
            ImGui::Text("%d%% crit", outcome.two_crit);
        ImGui::PopFont();
    }
    ImGui::End();

    // Health Window
	wf |= ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowSize(ImVec2(100, 100));
    ImGui::SetNextWindowPos(ImVec2(260, 440));
    ImGui::Begin("health1", NULL, wf);
    {
        //TextCentered(string text)
        ImGui::Text("%d", ally.health);
        ImGui::PushFont(uiFontMedium);
        ImGui::Text("HP");
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(100, 100));
    ImGui::SetNextWindowPos(ImVec2(510, 440));
    ImGui::Begin("health2", NULL, wf);
    {
        
        ImGui::Text("%d", target.health);
        ImGui::PushFont(uiFontMedium);
        ImGui::Text("HP");
        ImGui::PopFont();
    }
    ImGui::End();

	ImGui::PopFont(); // Large
}

// Displays options menu
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
DisplayTimer(ImGuiWindowFlags wf, const Timer &timer)
{
    ImGui::SetNextWindowPos(ImVec2(450, 30), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(60, 50));

	wf |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Timer", NULL, wf);
    {
        ImGui::PushFont(uiFontLarge);
            ImGui::Text("%02d", (int)((timer.end - timer.current) * 0.001));
        ImGui::PopFont();
    }
    ImGui::End();
}

void 
RenderUI(UI_State *ui, 
         const Cursor &cursor, 
         const Tilemap &map)
{
    if(GlobalEditorMode)
        return;
    if(!GlobalPlayerTurn)
        return;

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
		DisplayTileInfo(window_flags, map.tiles[cursor.pos.col][cursor.pos.row], cursor.Quadrant());
	if(ui->unit_blurb)
		DisplayUnitBlurb(window_flags, *map.tiles[cursor.pos.col][cursor.pos.row].occupant, cursor.Quadrant());
	if(ui->unit_info)
		DisplayUnitInfo(window_flags, *map.tiles[cursor.pos.col][cursor.pos.row].occupant, cursor.Quadrant());
	if(ui->combat_preview)
		DisplayCombatPreview(window_flags, *cursor.selected, *cursor.targeted, 
                                           map.tiles[cursor.selected->pos.col][cursor.selected->pos.row].avoid,
                                           map.tiles[cursor.pos.col][cursor.pos.row].avoid);
    if(ui->timer)
        DisplayTimer(window_flags, GlobalLevelTimer);
    if(ui->game_over)
        DisplayGameOver(window_flags);

	// cleanup
	ImGui::PopStyleVar();

	ImGui::PopStyleColor(3);
}

#endif
