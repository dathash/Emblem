// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Debug

#ifndef DEBUG_H
#define DEBUG_H

#if DEV_MODE

string 
GetInterfaceString(InterfaceState state)
{
    switch (state)
    {
    case NEUTRAL_OVER_GROUND:           return "Neutral over ground";
    case NEUTRAL_OVER_ENEMY:            return "Neutral over enemy";
    case NEUTRAL_OVER_UNIT:             return "Neutral over unit";
    case NEUTRAL_OVER_DEACTIVATED_UNIT: return "Neutral over deactivated unit";
    case SELECTED_OVER_GROUND:          return "Selected over ground";
    case SELECTED_OVER_INACCESSIBLE:    return "Selected over inaccessible";
    case SELECTED_OVER_ALLY:            return "Selected over ally";
    case SELECTED_OVER_ENEMY:           return "Selected over enemy";
    case ATTACK_TARGETING:              return "Attack targeting";
    case ABILITY_TARGETING:             return "Ability targeting";
    case GRAPPLE_TARGETING:             return "Grapple targeting";
    case TALK_TARGETING:                return "Talk targeting";
    case PREVIEW_ATTACK:                return "Preview attack";
    case PREVIEW_ABILITY:               return "Preview ability";
    case GAME_MENU:                     return "Game menu";
    case GAME_MENU_OUTLOOK:             return "Game menu outlook";
    case GAME_MENU_OPTIONS:             return "Game menu options";
    case ANIMATING_UNIT_MOVEMENT:       return "Animating unit movement";
    case UNIT_MENU_ROOT:                return "Unit menu root";
    case UNIT_INFO:                     return "Unit info";
    case ENEMY_INFO:                    return "Enemy info";
    case ENEMY_RANGE:                   return "Enemy range";
    case LEVEL_MENU:                    return "Level menu";
    case CONVERSATION_MENU:             return "Conversation menu";
    case CONVERSATION:                  return "Conversation";
    case BATTLE_CONVERSATION:           return "Battle conversation";
    case VILLAGE_CONVERSATION:          return "Village conversation";
    case PRELUDE:                       return "Prelude";
    case CUTSCENE:                      return "Cutscene";
    case PLAYER_FIGHT:                  return "Player fight";
    case RESOLVING_EXPERIENCE:          return "Resolving experience";
    case RESOLVING_ADVANCEMENT:         return "Resolving advancement";
    case DEATH:                         return "Death";
    case NO_OP:                         return "No-Op";
    case TITLE_SCREEN:                  return "Title screen";
    case GAME_OVER:                     return "Game over";
	default:
		cout << "WARNING GetInterfaceString " << state << "\n"; return "";
	}
}

string 
GetAIString(AIState state)
{
    switch (state)
    {
    case AI_FINDING_NEXT:           return "AI Finding Next";
    case AI_SELECTED:               return "AI Selected";
    case AI_RESOLVING_EXPERIENCE:   return "AI Resolving Experience";
    case AI_RESOLVING_ADVANCEMENT:  return "AI Resolving Advancement";
    case AI_FIGHT:                  return "AI Fight";
    case AI_PLAYER_TURN:            return "AI Player Turn";
    case AI_DEATH:                  return "AI Death";
    case AI_NO_OP:                  return "AI No-op";
	default:
		cout << "WARNING GetAIString " << state << "\n"; return "";
    }
}

void
DebugUI()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoInputs;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT));

	ImGui::PushFont(uiFontLarge);
    ImGui::PushStyleColor(ImGuiCol_Text, SdlToImColor(white));
    ImGui::Begin("Debug", NULL, flags);
    {
        ImGui::Text("[%.2f FPS]", ImGui::GetIO().Framerate);
        ImGui::Text("%s", GetInterfaceString(GlobalInterfaceState).c_str());
        ImGui::Text("%s", GetAIString(GlobalAIState).c_str());
        ImGui::Text("Viewport [%d, %d]", viewportCol, viewportRow);
        ImGui::Text("Speed 1/%d", GlobalSpeedMod);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

#endif
#endif
