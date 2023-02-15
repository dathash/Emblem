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
    case NO_OP:                         return "No-Op";
    case TITLE_SCREEN:                  return "Title screen";
    case GAME_OVER:                     return "Game over";
    case GAME_MENU:                     return "Game menu";
    case GAME_MENU_OPTIONS:             return "Game menu options";
    case ENEMY_RANGE:                   return "Enemy range";
    case NEUTRAL_OVER_GROUND:           return "Neutral over ground";
    case NEUTRAL_OVER_ENEMY:            return "Neutral over enemy";
    case NEUTRAL_OVER_UNIT:             return "Neutral over unit";
    case NEUTRAL_OVER_DEACTIVATED_UNIT: return "Neutral over deactivated unit";
    case SELECTED:                      return "Selected";
    case ATTACK_TARGETING:              return "Attack targeting";
    case ATTACK_RESOLUTION:             return "Attack resolution";
	default:
		cout << "WARNING GetInterfaceString " << state << "\n"; return "";
	}
}

string 
GetAIString(AIState state)
{
    switch (state)
    {
    case AI_NO_OP:                  return "AI No-op";
    case AI_FINDING_NEXT:           return "AI Finding Next";
    case AI_SELECTED:               return "AI Selected";
    case AI_ATTACK_RESOLUTION:      return "AI Attack Resolution";
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
        ImGui::Text("Speed 1/%d", GlobalSpeedMod);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

#endif
#endif
