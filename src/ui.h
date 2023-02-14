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

string
GetObjectiveString(Objective obj)
{
    switch (obj)
    {
    case OBJECTIVE_ROUT: return "Rout";
    case OBJECTIVE_CAPTURE: return "Capture";
    case OBJECTIVE_BOSS: return "Boss";
	default:
		assert(!"ERROR: Unhandled Objective string in UI.\n");
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
    bool outlook = false;
    bool unit_blurb = false;
    bool unit_info = false;
    bool combat_preview = false;
    bool game_menu = false;
    bool game_over = false;
    bool unit_menu = false;
    bool combat_screen = false;
    bool death_screen = false;
    bool experience = false;
    bool advancement = false;

    void
    Clear()
    {
        tile_info = false;
        outlook = false;
        unit_blurb = false;
        unit_info = false;
        combat_preview = false;
        game_menu = false;
        game_over = false;
        unit_menu = false;
        combat_screen = false;
        death_screen = false;
        experience = false;
        advancement = false;
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

        if(GlobalInterfaceState == DEATH ||
           GlobalAIState == AI_DEATH)
        {
            death_screen = true;
        }
        else
        {
            death_screen = false;
        }

        if(GlobalInterfaceState == RESOLVING_EXPERIENCE ||
           GlobalAIState == AI_RESOLVING_EXPERIENCE)
        {
            experience = true;
        }
        else
        {
            experience = false;
        }

        if(GlobalInterfaceState == RESOLVING_ADVANCEMENT ||
           GlobalAIState == AI_RESOLVING_ADVANCEMENT)
        {
            advancement = true;
        }
        else
        {
            advancement = false;
        }

		// Tile Info
        if(
            GlobalInterfaceState == NEUTRAL_OVER_GROUND || 
            GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
            GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
            GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
            GlobalInterfaceState == SELECTED_OVER_GROUND ||
            GlobalInterfaceState == SELECTED_OVER_INACCESSIBLE ||
            GlobalInterfaceState == SELECTED_OVER_ALLY ||
            GlobalInterfaceState == SELECTED_OVER_ENEMY ||
            GlobalInterfaceState == ATTACK_TARGETING ||
            GlobalInterfaceState == ABILITY_TARGETING ||
            GlobalInterfaceState == ENEMY_INFO ||
            GlobalInterfaceState == UNIT_MENU_ROOT
			)
        {
            tile_info = true;
        }
        else
        {
            tile_info = false;
        }

        if(GlobalInterfaceState == GAME_MENU_OUTLOOK)
        {
            outlook = true;
        }
        else
        {
            outlook = false;
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
DisplayOutlook(ImGuiWindowFlags wf, const Level &level)
{
	// Window sizing
    ImGui::SetNextWindowPos(ImVec2(340, 200));
    ImGui::SetNextWindowSize(ImVec2(360, 160));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Outlook", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
			ImGui::Text("Objective - %s", GetObjectiveString(level.objective).c_str());
			ImGui::Text("Enemies - %d", level.GetNumberOf(false));
			ImGui::Text("Allies  - %d", level.GetNumberOf(true));
		ImGui::PopFont();
    }
    ImGui::End();

    return;
}

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

			DisplayHealthBar(unit.health, unit.MaxHealth(), 0);

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);
			ImGui::Text("%d / %d", unit.health, unit.MaxHealth());
		ImGui::PopFont();
    }
    ImGui::End();
}

void
DisplayItemInfo(Item *item, int bonus_damage = 0)
{
    if(item->weapon)
    {
        ImGui::Text("[%s, %d%s+%d wt [%d-%d]]", 
                    GetItemString(item->type).c_str(),
                    item->weapon->num_dice,
                    GetDieString(item->weapon->die).c_str(),
                    bonus_damage,
                    item->weapon->min_range, item->weapon->max_range);
    }
    if(item->consumable)
    {
        ImGui::Text("[%s, %d, %d]", 
                    GetItemString(item->type).c_str(),
                    item->consumable->amount,
                    item->consumable->uses);
    }
    if(item->equipment)
    {
        ImGui::Text("[%s, %d]", 
                    GetItemString(item->type).c_str(),
                    item->consumable->amount);
    }
}

// Full display of unit's stats
void 
DisplayUnitInfo(ImGuiWindowFlags wf, const Unit &unit, enum quadrant quad)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(480, 200));

    int x_pos = 410;
    if(!unit.is_ally)
        x_pos = 170;

    ImVec2 top_right = ImVec2(x_pos, 10);
    ImVec2 bottom_right = ImVec2(x_pos, 390);

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

            if(unit.is_boss)
            {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(red));
                ImGui::Text("BOSS");
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            ImGui::Text("lv %d+%d xp", unit.level, unit.experience);

            ImGui::SameLine(ImGui::GetWindowWidth() - 120);
			ImGui::Text("[%d/%d]", unit.health, unit.MaxHealth());

		ImGui::PopFont();
		ImGui::PushFont(uiFontSmall);

			DisplayHealthBar(unit.health, unit.MaxHealth(), 0);

			// Second line
            int strength = unit.strength;
            if(unit.buff && unit.buff->stat == STAT_ATTACK)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                strength += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkRed));
            }
			ImGui::Text("[STR %d]", strength);
            ImGui::PopStyleColor();

            int dexterity = unit.dexterity;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_DEFENSE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                dexterity += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkBlue));
            }
			ImGui::Text("[DEX %d]", dexterity);
            ImGui::PopStyleColor();

            int vitality = unit.vitality;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_DEFENSE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                vitality += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(darkGreen));
            }
			ImGui::Text("[VIT %d]", vitality);
            ImGui::PopStyleColor();

            int intuition = unit.intuition;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_APTITUDE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                intuition += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(purple));
            }
			ImGui::Text("[INT %d]", intuition);
            ImGui::PopStyleColor();

            int faith = unit.faith;
			ImGui::SameLine();
            if(unit.buff && unit.buff->stat == STAT_SPEED)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                faith += unit.buff->amount;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(yellow));
            }
			ImGui::Text("[FTH %d]", faith);
            ImGui::PopStyleColor();

            if(unit.weapon)
            {
                DisplayItemInfo(unit.weapon, unit.GetWeaponDmgStat());
            }
            if(unit.pocket)
            {
                DisplayItemInfo(unit.pocket);
            }

			ImGui::Text("[MOV %d]", unit.Movement());
			ImGui::SameLine();
			ImGui::Text("[%d AC]", unit.AC());
			ImGui::SameLine();
			ImGui::Text("[+%d to hit]", unit.ToHit());
			ImGui::SameLine();
			ImGui::Text("[CRIT-%d]", unit.Crit());

            if(unit.ability != ABILITY_NONE)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(accentBlue));
                ImGui::Text("[%s]", GetAbilityString(unit.ability).c_str());
                ImGui::PopStyleColor();
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
            ImGui::Text("%d%s+%d", outcome.num_dice, GetDieString(outcome.die).c_str(), outcome.bonus_damage);
            ImGui::Text("target-%d", outcome.target);
            ImGui::Text("crit-%d", outcome.crit);
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(350, 160));
    ImGui::SetNextWindowPos(ImVec2(510, 400));

    ImGui::Begin(target.name.c_str(), NULL, wf);
    {
		ImGui::PushFont(uiFontMedium);
            ImGui::SameLine(ImGui::GetWindowWidth()-150);
            ImGui::Text("--");
            ImGui::NewLine();
            ImGui::SameLine(ImGui::GetWindowWidth()-150);
            ImGui::Text("--");
            ImGui::NewLine();
            ImGui::SameLine(ImGui::GetWindowWidth()-150);
            ImGui::Text("--");
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
        int health_after = clamp(ally.health, 0, ally.MaxHealth());
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)ally.health / ally.MaxHealth())));
        ImGui::Text("%d", ally.health);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)health_after / ally.MaxHealth())));
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
        int health_after = clamp(target.health - (outcome.num_dice * GetMaxValue(outcome.die) + outcome.bonus_damage), 0, target.MaxHealth());
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)target.health / target.MaxHealth())));
        ImGui::Text("%d", target.health);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(PiecewiseColors(red, yellow, green, (float)health_after / target.MaxHealth())));
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

// Displays Death quote
void
DisplayDeathScreen(ImGuiWindowFlags wf, Unit *dying)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(800, 100));
    ImGui::SetNextWindowPos(ImVec2(20, 500));

    // Render
    ImGui::PushFont(uiFontMedium);
    ImGui::Begin(dying->name.c_str(), NULL, wf);
    {
        ImGui::Text("%s", dying->valediction.c_str());
    }
    ImGui::End();
    ImGui::PopFont();
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
        DisplayHealthBar(fight.one->health, fight.one->MaxHealth(), 0);
		ImGui::PushFont(uiFontMedium);
        ImGui::PopFont();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(300, 110));
    ImGui::SetNextWindowPos(ImVec2(540, y_pos));

    ImGui::Begin(fight.two->name.c_str(), NULL, wf);
    {
        DisplayHealthBar(fight.two->health, fight.two->MaxHealth(), 0);
		ImGui::PushFont(uiFontMedium);
        ImGui::PopFont();
    }
    ImGui::End();

	ImGui::PopFont(); // Large
}


void 
DisplayExperienceBar(int experience)
{
	float percent = ((float)experience / 100.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 0, 255));
	ImGui::ProgressBar(percent, ImVec2(-1.0f, 0.0f));
	ImGui::PopStyleColor();
}

// Displays Experience bar over its parcelling
void
DisplayExperience(ImGuiWindowFlags wf, const Parcel &parcel)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(350, 150));
    ImGui::SetNextWindowPos(ImVec2(300, 150));

    // Render
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(parcel.recipient->name.c_str(), NULL, wf);
    {
        DisplayExperienceBar(parcel.Amount());
        TextCentered("Experience");
    }
    ImGui::End();
	ImGui::PopFont();
}

// Displays combat preview when initiating combat
void
DisplayAdvancement(ImGuiWindowFlags wf, const Advancement &advancement)
{
	// Window sizing
    ImGui::SetNextWindowSize(ImVec2(300, 400));
    ImGui::SetNextWindowPos(ImVec2(500, 100));

    // Render
	ImGui::PushFont(uiFontLarge);
    ImGui::Begin(advancement.recipient->name.c_str(), NULL, wf);
    {
        Unit *subject = advancement.recipient;

        ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(cerulean));
        TextCentered("Level Up!");
        ImGui::PopStyleColor();
    }
    ImGui::End();
	ImGui::PopFont();
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
         const Level &level,
         const Fight &fight,
         const Parcel &parcel,
         const Advancement &advancement,
         Unit *dying
         )
{
    ui->Update();

    if(GlobalEditorMode || GlobalDebug) // For now
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

    // TITLE SCREEN
    if(GlobalInterfaceState == TITLE_SCREEN)
    {
        DisplayTitleScreen(window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return;
    }

    // USER/AI UI
    if(ui->combat_screen)
		DisplayCombatScreen(window_flags, fight);
    if(ui->death_screen)
        DisplayDeathScreen(window_flags, dying);
    if(ui->experience)
        DisplayExperience(window_flags, parcel);
    if(ui->advancement)
        DisplayAdvancement(window_flags, advancement);

    if(!GlobalPlayerTurn)
    {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return;
    }

    // USER MODE UI
    if(ui->outlook)
        DisplayOutlook(window_flags, level);
	if(ui->tile_info)
		DisplayTileInfo(window_flags, level.map.tiles[cursor.pos.col][cursor.pos.row], Quadrant(cursor.pos));
	if(ui->unit_blurb)
		DisplayUnitBlurb(window_flags, *level.map.tiles[cursor.pos.col][cursor.pos.row].occupant, Quadrant(cursor.pos));
	if(ui->unit_info)
		DisplayUnitInfo(window_flags, *level.map.tiles[cursor.pos.col][cursor.pos.row].occupant, Quadrant(cursor.pos));
	if(ui->combat_preview)
		DisplayCombatPreview(window_flags, *cursor.selected, *cursor.targeted, 
                                           level.map.tiles[cursor.selected->pos.col][cursor.selected->pos.row].avoid,
                                           level.map.tiles[cursor.pos.col][cursor.pos.row].avoid,
                                           level.map.tiles[cursor.selected->pos.col][cursor.selected->pos.row].defense,
                                           level.map.tiles[cursor.pos.col][cursor.pos.row].defense
                                           );
    if(ui->game_over)
        DisplayGameOver(window_flags);

	// cleanup
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
}

#endif
