// Author: Alex Hartford                                                                                 
// Program: Emblem
// File: Debug

#ifndef DEBUG_H
#define DEBUG_H

#if DEV_MODE

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

	ImGui::PushFont(uiFontSmall);
    ImGui::Begin("Debug", NULL, flags);
    {
        ImGui::Text("[%.2f FPS]", ImGui::GetIO().Framerate);
        ImGui::Text("Phase: %s", GetPhaseString(GlobalPhase).c_str());
        ImGui::Text("%s", GetInterfaceString(GlobalInterfaceState).c_str());
        ImGui::Text("%s", GetAIString(GlobalAIState).c_str());
        ImGui::Text("Speed 1/%d", GlobalSpeedMod);
    }
    ImGui::End();
    ImGui::PopFont();
}

#endif
#endif
