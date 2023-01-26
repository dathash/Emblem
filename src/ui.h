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
    case SPAWN: return "Spawn";
    case FORT: return "Fort";
	default:
		assert(!"ERROR: Unhandled Tile name string in UI.\n");
		return "";
	}
}

string 
GetInterfaceString(InterfaceState state)
{
    switch (state)
    {
    case NO_OP: return "No-Op";
                // TODO: Fill these in!!
	default:
		assert(!"ERROR: Unhandled InterfaceState name string in UI.\n");
		return "";
	}
}

string 
GetAbilityString(Ability ability)
{
    switch (ability)
    {
    case ABILITY_NONE: return "None";
    case ABILITY_HEAL: return "Heal";
    case ABILITY_BUFF: return "Buff";
    case ABILITY_SHIELD: return "Shield";
    case ABILITY_DANCE: return "Dance";
	default:
		assert(!"ERROR: Unhandled Ability name string in UI.\n");
		return "";
	}
}

string
GetStatString(Stat stat)
{
    switch (stat)
    {
    case STAT_NONE: return "N/A";
    case STAT_ATTACK: return "Atk";
    case STAT_DEFENSE: return "Def";
    case STAT_APTITUDE: return "Apt";
    case STAT_SPEED: return "Spd";
	default:
		assert(!"ERROR: Unhandled Ability name string in UI.\n");
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
    }

    void 
    Update()
    {
        if(GlobalInterfaceState == PLAYER_FIGHT ||
           GlobalAIState == AI_FIGHT)
        {
            combat_screen = true;
        }
        else
        {
            combat_screen = false;
        }

		// Tile Info
        if(
           !(GlobalInterfaceState == LEVEL_MENU ||
             GlobalInterfaceState == CONVERSATION ||
             GlobalInterfaceState == PREVIEW_ATTACK ||
             GlobalInterfaceState == PREVIEW_ABILITY ||
             GlobalInterfaceState == PLAYER_FIGHT)
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
				GlobalInterfaceState == ABILITY_TARGETING ||
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
		if(GlobalInterfaceState == UNIT_INFO ||
		   GlobalInterfaceState == ENEMY_INFO)
		{
			unit_info = true;
		}
		else
		{
			unit_info = false;
		}

		// Combat Preview
		if(GlobalInterfaceState == PREVIEW_ATTACK ||
		   GlobalInterfaceState == PREVIEW_ABILITY)
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
    ImGui::Begin(GetTileNameString(tile.type).c_str(), NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			ImGui::Text("hide %d", tile.avoid);
			ImGui::Text("def  %d", tile.defense);
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

            if(unit.buff)
            {
                ImGui::SameLine(ImGui::GetWindowWidth()-220);
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                ImGui::Text("[BUFF: %s/%d]", GetStatString(unit.buff->stat).c_str(), 
                                               unit.buff->turns_remaining);
                ImGui::PopStyleColor();
            }

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);

			DisplayHealthBar(unit.health, unit.max_health, 0);

			// Second line
            int attack = unit.attack;
            if(unit.buff && unit.buff->stat == STAT_ATTACK)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                attack += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkRed));
            }
			ImGui::Text("[ATK %d]", attack);
            ImGui::PopStyleColor();

            int defense = unit.defense;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_DEFENSE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                defense += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkBlue));
            }
			ImGui::Text("[DEF %d]", defense);
            ImGui::PopStyleColor();

            int aptitude = unit.aptitude;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_APTITUDE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                aptitude += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkGreen));
            }
			ImGui::Text("[APT %d]", aptitude);
            ImGui::PopStyleColor();

            int speed = unit.speed;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_SPEED)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                speed += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(yellow));
            }
			ImGui::Text("[SPD %d]", speed);
            ImGui::PopStyleColor();

            if(unit.ability != ABILITY_NONE)
            {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                ImGui::Text("[%s]", GetAbilityString(unit.ability).c_str());
                ImGui::PopStyleColor();
            }


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
                                          int ally_avoid_bonus, int enemy_avoid_bonus,
                                          int ally_defense_bonus, int enemy_defense_bonus)
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
                  ally_avoid_bonus, enemy_avoid_bonus,
                  ally_defense_bonus, enemy_defense_bonus);
    }

	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(350, 160));
    ImGui::SetNextWindowPos(ImVec2(50, 400));

    // Render
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(ally.name.c_str(), NULL, wf);
    {
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("%d dmg", outcome.one_damage);
            if(outcome.one_doubles)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                ImGui::SameLine();
                ImGui::Text("X2");
                ImGui::PopStyleColor();
            }
            ImGui::Text("%d%% hit", outcome.one_hit);
            ImGui::Text("%d%% crit", outcome.one_crit);
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(350, 160));
    ImGui::SetNextWindowPos(ImVec2(510, 400));

    ImGui::Begin(target.name.c_str(), NULL, wf);
    {
		ImGui::PushFont(uiFontMedium);
            if(outcome.two_attacks)
            {
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("%d dmg", outcome.two_damage);
                if(outcome.two_doubles)
                {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                    ImGui::Text("X2");
                    ImGui::PopStyleColor();
                }
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("%d%% hit", outcome.two_hit);
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("%d%% crit", outcome.two_crit);
            }
            else
            {
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("-- dmg");
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("--%% hit");
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetWindowWidth()-150);
                ImGui::Text("--%% crit");
            }
        ImGui::PopFont();
    }
    ImGui::End();

    // Health Window
	wf |= ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowSize(ImVec2(170, 100));
    ImGui::SetNextWindowPos(ImVec2(225, 450));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, SdlToImColor(uiDarkColor));
    ImGui::Begin("health1", NULL, wf);
    {
        int health_after = clamp(ally.health - (outcome.two_doubles ? 2 * outcome.two_damage : outcome.two_damage), 
                                 0, ally.max_health);
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)ally.health / ally.max_health)));
        ImGui::Text("%d", ally.health);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)health_after / ally.max_health)));
        ImGui::Text("%d", health_after);
        ImGui::PopStyleColor();
        ImGui::PushFont(uiFontMedium);
        ImGui::Text("HP");
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleColor();

    ImGui::SetNextWindowSize(ImVec2(170, 100));
    ImGui::SetNextWindowPos(ImVec2(515, 450));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, SdlToImColor(uiDarkColor));
    ImGui::Begin("health2", NULL, wf);
    {
        int health_after = clamp(target.health - (outcome.one_doubles ? 2 * outcome.one_damage : outcome.one_damage), 
                                 0, target.max_health);
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)target.health / target.max_health)));
        ImGui::Text("%d", target.health);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)health_after / target.max_health)));
        ImGui::Text("%d", health_after);
        ImGui::PopStyleColor();
        ImGui::PushFont(uiFontMedium);
        ImGui::NewLine();
        ImGui::SameLine(ImGui::GetWindowWidth()-50);
        ImGui::Text("HP");
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleColor();

	ImGui::PopFont(); // Large
}

// Displays combat preview when initiating combat
void
DisplayCombatScreen(ImGuiWindowFlags wf, const Fight &fight)
{
    int y_pos = 400;
    if(fight.lower_half_screen)
    {
        y_pos = 100;
    }
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(300, 110));
    ImGui::SetNextWindowPos(ImVec2(50, y_pos));

    // Render
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(fight.one->name.c_str(), NULL, wf);
    {
        DisplayHealthBar(fight.one->health, fight.one->max_health, 0);
		ImGui::PushFont(uiFontMedium);
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(300, 110));
    ImGui::SetNextWindowPos(ImVec2(540, y_pos));

    ImGui::Begin(fight.two->name.c_str(), NULL, wf);
    {
        DisplayHealthBar(fight.two->health, fight.two->max_health, 0);
		ImGui::PushFont(uiFontMedium);
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
RenderUI(UI_State *ui, 
         const Cursor &cursor, 
         const Tilemap &map,
         const Fight &fight)
{
    if(GlobalEditorMode)
        return;

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

    // USER/AI UI
    if(ui->combat_screen)
		DisplayCombatScreen(window_flags, fight);

    if(!GlobalPlayerTurn)
    {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return;
    }

    // USER MODE UI
	if(ui->tile_info)
		DisplayTileInfo(window_flags, map.tiles[cursor.pos.col][cursor.pos.row], Quadrant(cursor.pos));
	if(ui->unit_blurb)
		DisplayUnitBlurb(window_flags, *map.tiles[cursor.pos.col][cursor.pos.row].occupant, Quadrant(cursor.pos));
	if(ui->unit_info)
		DisplayUnitInfo(window_flags, *map.tiles[cursor.pos.col][cursor.pos.row].occupant, Quadrant(cursor.pos));
	if(ui->combat_preview)
		DisplayCombatPreview(window_flags, *cursor.selected, *cursor.targeted, 
                                           map.tiles[cursor.selected->pos.col][cursor.selected->pos.row].avoid,
                                           map.tiles[cursor.pos.col][cursor.pos.row].avoid,
                                           map.tiles[cursor.selected->pos.col][cursor.selected->pos.row].defense,
                                           map.tiles[cursor.pos.col][cursor.pos.row].defense
                                           );
    if(ui->game_over)
        DisplayGameOver(window_flags);

	// cleanup
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
}

#endif
