// Author: Alex Hartford
// Program: Emblem
// File: Main

// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>

using namespace std;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef pair<int, int> point;
typedef vector<point> path;

#include "constants.h"

// ============================= globals ===================================
static InterfaceState GlobalInterfaceState = NEUTRAL_OVER_GROUND;
static AIState GlobalAIState = ENEMY_TURN;

static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;
static ma_engine GlobalMusicEngine;

static bool GlobalRunning = false;

static int GlobalLevelNumber = 0;
static bool GlobalNextLevel = false;
static int GlobalTotalLevels = 10;

static bool GlobalPlayerTurn = true;
static bool GlobalTurnStart = true;

static bool GlobalGamepadMode = false;

static bool GlobalEditorMode = false;
static int GlobalCurrentID = 0;

static int viewportCol = 0;
static int viewportRow = 0;

// TODO: unnecessary?
static ImFont *uiFontSmall;
static ImFont *uiFontMedium;
static ImFont *uiFontLarge;

// TODO: unnecessary?
static u64 GlobalFrameNumber = 0;

static point leaderPosition = {0, 0};

#include "utils.h"
#include "structs.h"
#include "load.h"
#include "init.h"
#include "input.h"
#include "grid.h"
#include "fight.h"
#include "ui.h"
#include "command.h"
#include "ai.h"
#include "render.h"
#include "editor.h"

int main(int argc, char *argv[])
{
// ================================== init =================================
    srand(time(NULL));

    if(!Initialize())
        assert(!"Initialization Failed\n");

    // controller init
    SDL_Joystick *gamePad = NULL;
    if(SDL_NumJoysticks() > 0)
    {
        GlobalGamepadMode = true;
        gamePad = SDL_JoystickOpen(0);
        assert(gamePad);
    }

// ================================== load =================================
    vector<unique_ptr<Unit>> units = LoadUnits(DATA_PATH + string(INITIAL_UNITS));
    vector<string> levels = {DATA_PATH + string("l0.txt"), DATA_PATH + string("l1.txt"),
                             DATA_PATH + string("l2.txt"), DATA_PATH + string("l3.txt"),
                             DATA_PATH + string("l4.txt"), DATA_PATH + string("l5.txt"),
                             DATA_PATH + string("l6.txt"), DATA_PATH + string("l7.txt")
							 };
    Level level = LoadLevel(levels[GlobalLevelNumber], units);
    Cursor cursor(SpriteSheet(LoadTextureImage(SPRITES_PATH, string("cursor.png")), 
		32, ANIMATION_SPEED));

	UI_State ui = {};

    // Initialize Menus
    Menu gameMenu(3, 0, {"Outlook", "Options", "End Turn"});
    Menu unitMenu(1, 0, {"Wait"});

    // initial actor state
    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    // frame timer
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime = 0;
    float elapsedMS = 0.0f;

	// Play Music
    //ma_engine_play_sound(&GlobalMusicEngine, "../assets/audio/r4.wav", NULL);

// ========================= game loop =========================================
    GlobalRunning = true;
    while(GlobalRunning)
    {
        HandleEvents(&input);
// ====================== command phase ========================================
        if(GlobalTurnStart)
        {
            for(auto const &unit : level.combatants)
                unit->isExhausted = false;
			cursor.Reset();
			viewportCol = 0; // TODO: Now this is a lie!
            viewportRow = 0;
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;

            GlobalTurnStart = false;
            ai.clearQueue();
            handler.clearQueue();
        }

		if(GlobalPlayerTurn)
		{
			handler.Update(&input);
			handler.UpdateCommands(&cursor, &level.map,
								   &gameMenu, &unitMenu);
		}
		else
		{
			if(ai.shouldPlan)
				ai.Plan(&cursor, &level.map);
			if(!(GlobalFrameNumber % 10))
				ai.Update();
		}

// ========================= update phase =======================================
        cursor.Update();
		ui.Update();

        for(auto const &unit : level.combatants)
            unit->Update();

        // cleanup
        level.RemoveDeadUnits();

        if(GlobalNextLevel)
        {
            GlobalNextLevel = false;
            GlobalLevelNumber = (GlobalLevelNumber + 1 < GlobalTotalLevels) ? 
				GlobalLevelNumber + 1 : 0;
			leaderPosition = {0, 0};
            level = LoadLevel(levels[GlobalLevelNumber], units);
			cursor.Reset();
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;
        }

// ============================= render =========================================
        Render(level.map, cursor, gameMenu, unitMenu);

        // IMGUI
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if(GlobalPlayerTurn)
			RenderUI(&ui, cursor, level.map);
#if DEV_MODE
        if(GlobalEditorMode)
            EditorPass(&units, &level);
#endif
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(GlobalRenderer);

// =========================== timing ============================================
        endTime = SDL_GetPerformanceCounter();
        elapsedMS = ((endTime - startTime) / 
					 (float)SDL_GetPerformanceFrequency() * 1000.0f);

		// Set constant 60fps
        if(elapsedMS < MS_PER_FRAME)
            SDL_Delay((int)(MS_PER_FRAME - elapsedMS));

        startTime = SDL_GetPerformanceCounter();
        GlobalFrameNumber++;
    } // End Game Loop

    Close();
    return 0;
}
