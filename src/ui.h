// Author: Alex Hartford
// Program: Emblem
// File: UI

#ifndef UI_H
#define UI_H

void 
TextCentered(string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text.c_str());
}

struct UI_State
{
    bool tile_info = false;
    bool unit_blurb = false;
    bool unit_selected = false;
    bool attack_targeting = false;
    bool game_menu = false;
    bool game_over = false;
    bool victory = false;
    bool unit_menu = false;

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
            GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT
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
        if(GlobalInterfaceState == ATTACK_TARGETING ||
           GlobalInterfaceState == ATTACK_THINKING)
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
		if(GlobalInterfaceState == VICTORY)
		{
			victory = true;
		}
		else
		{
			victory = false;
		}
    }
};

void
DisplayTitleScreen(ImGuiWindowFlags wf)
{
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
    ImGui::SetNextWindowSize(ImVec2(150, 80));
    ImGui::SetNextWindowPos(
            ImVec2(X_OFFSET + TILE_SIZE * MAP_WIDTH + 10, 
                   Y_OFFSET + TILE_SIZE * MAP_HEIGHT - 90));

    ImGui::PushFont(uiFontMedium);
    ImGui::Begin(GetTileTypeString(tile.type).c_str(), NULL, wf);
    {
    }
    ImGui::End();
    ImGui::PopFont();
}

void 
DisplayTurnCount(ImGuiWindowFlags wf, int turn, int victory_turn)
{
    ImGui::SetNextWindowSize(ImVec2(100, 80));
    ImGui::SetNextWindowPos(
            ImVec2(X_OFFSET + TILE_SIZE * MAP_WIDTH + 170,
                   Y_OFFSET + TILE_SIZE * MAP_HEIGHT - 90));

    ImGui::PushFont(uiFontMedium);
    ImGui::Begin("Left", NULL, wf);
    {
        ImGui::Text("%d", victory_turn - turn);
    }
    ImGui::End();
    ImGui::PopFont();
}

// Displays a health bar, with an overlay for the damage to be done.
void 
DisplayHealthBar(int health, int max_health, int damage = 0)
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
    ImGui::SetNextWindowSize(ImVec2(250, 180));
    ImGui::SetNextWindowPos(
            ImVec2(X_OFFSET + TILE_SIZE * MAP_WIDTH + 10, 
                   160));

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
            if(unit.healing)
            {
                ImGui::SameLine();
                DisplayEquipInfo(unit.healing);
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
    ImGui::SetNextWindowPos(ImVec2(400, 300), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(420, 310));

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
DisplayVictory(ImGuiWindowFlags wf)
{
    ImGui::SetNextWindowPos(ImVec2(400, 300), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(500, 200));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Victory", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("You won!");
            ImGui::Text("[SPACE] to play next level");
            ImGui::Text("[SHIFT] to quit");
		ImGui::PopFont();
    }
    ImGui::End();
}

void
DisplayPlayerState(ImGuiWindowFlags wf, const Player &player)
{
    ImGui::SetNextWindowSize(ImVec2(250, 140));
    ImGui::SetNextWindowPos(
            ImVec2(X_OFFSET + TILE_SIZE * MAP_WIDTH + 10, 
                   10));


	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Player", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
            DisplayHealthBar(player.health, player.max_health);
            ImGui::Text("Health: %d/%d", player.health, player.max_health);
		ImGui::PopFont();
    }
    ImGui::End();
}

void
DisplayTurnQueue(ImGuiWindowFlags wf)
{
    ImGui::SetNextWindowSize(ImVec2(180, 400));
    ImGui::SetNextWindowPos(ImVec2(10, 10));


	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Queue", NULL, wf);
    {
    }
    ImGui::End();
    ImGui::PopFont();
}

void
DisplayUndoPrompt(ImGuiWindowFlags wf)
{
    ImGui::SetNextWindowSize(ImVec2(400, 200));
    ImGui::SetNextWindowPos(ImVec2(320, 200));

	ImGui::PushFont(uiFontLarge);
    ImGui::Begin("Undo", NULL, wf);
    {
		ImGui::PopFont();
		ImGui::PushFont(uiFontMedium);
            ImGui::Text("Really Undo?");
            ImGui::Text("You only have one.");
		ImGui::PopFont();
    }
    ImGui::End();
}

void 
RenderUI(UI_State *ui, 
         const Cursor &cursor, 
         const Level &level,
         const Player &player
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

    DisplayPlayerState(window_flags, GlobalPlayer);
    DisplayTurnCount(window_flags, level.turn_count, level.victory_turn);

    if(GlobalPhase != PHASE_PLAYER)
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
    if(ui->victory)
        DisplayVictory(window_flags);

    if(GlobalInterfaceState == GAME_MENU_QUEUE)
        DisplayTurnQueue(window_flags);
    if(GlobalInterfaceState == GAME_MENU_UNDO)
        DisplayUndoPrompt(window_flags);

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
}

#endif
