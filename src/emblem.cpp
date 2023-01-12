// Author: Alex Hartford
// Program: Emblem
// File: Main

// ========================== includes =====================================
// Library includes
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

// Local external files
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <iostream>

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

// ============================= globals ===================================
// SDL
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;

// IMGUI
static ImFont *uiFontSmall;
static ImFont *uiFontMedium;
static ImFont *uiFontLarge;

// MINIAUDIO
static ma_engine GlobalMusicEngine;

//// STATE //////////////////////
// TODO: consolidate
// Necessary State:
// * Running
// * Editor / Paused
// * PlayerTurn ? (We could maybe just store this in the InterfaceState at any
//                 given time)
// Transitories:
// * StartPlayerTurn
// * StartEnemyTurn
// * Resetting
// * Next Level
// * Animating
static bool GlobalRunning = false;
static bool GlobalEditorMode = false;
static bool GlobalPlayerTurn = true;

// Transitory
static bool GlobalNextLevel = false;
static bool GlobalRestart = false;
static bool GlobalTurnStart = true;

static unsigned int GlobalInterfaceState;
static unsigned int GlobalAIState;
////////////////////////////////

// TODO: Refactor these to only be where they need to be.
// Pretty sure none of them have to be globals.
static int viewportCol = 0;
static int viewportRow = 0;
static int GlobalCurrentID = 0;

// ================================= my includes ===============================
// NOTE: This is a unity build. This is all that my game includes.
#include "constants.h"
#include "utils.h"
#include "state.h"
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
    srand(time(NULL));

    if(!Initialize())
        assert(!"Initialization Failed\n");

    // controller init
    SDL_Joystick *gamepad = NULL;
    if(SDL_NumJoysticks() > 0)
    {
        gamepad = SDL_JoystickOpen(0);
        assert(gamepad);
    }

// ================================== load =================================
    vector<unique_ptr<Unit>> units = LoadUnits(DATA_PATH + string(INITIAL_UNITS));

    // Level
    vector<string> levels = {DATA_PATH + string("l0.txt"), DATA_PATH + string("l1.txt"),
                             DATA_PATH + string("l2.txt"), DATA_PATH + string("l3.txt"),
                             DATA_PATH + string("l4.txt"), DATA_PATH + string("l5.txt"),
                             DATA_PATH + string("l6.txt"), DATA_PATH + string("l7.txt")
							 };
    int level_index = 0;
    Level level = LoadLevel(levels[level_index], units);
    Cursor cursor(SpriteSheet(LoadTextureImage(SPRITES_PATH, string("cursor.png")), 
		32, ANIMATION_SPEED));

	UI_State ui = {};

    // Menus
    Menu gameMenu(3, 0, {"Outlook", "Options", "End Turn"});
    Menu unitMenu(1, 0, {"Wait"});

    // Actor state
    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    GlobalInterfaceState = NO_OP;
    GlobalAIState = PLAYER_TURN;

	// Play Music
    //ma_engine_play_sound(&GlobalMusicEngine, "../assets/audio/r4.wav", NULL);

    // Initialize state
    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        HandleEvents(&input, gamepad);
// ========================= update phase =======================================
        if(!GlobalEditorMode)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level.map,
                                   &gameMenu, &unitMenu);

            ai.Update(&cursor, &level.map);

            cursor.Update();
            ui.Update();
            level.Update();

            for(const unique_ptr<Unit> &unit : level.combatants)
                unit->Update();
        }
// ====================== state resolution phase ===============================
        //////////////// TO BE EXTRICATED //////////////////
        if(GlobalRestart)
        {
            GlobalRestart = false;
            --level_index;
            NextLevel();
        }

        if(GlobalNextLevel)
        {
            GlobalNextLevel = false;
            level_index = (level_index + 1 < TOTAL_LEVELS) ? level_index + 1 : 0;
            level = LoadLevel(levels[level_index], units);
            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
        }

        if(GlobalTurnStart)
        {
            GlobalTurnStart = false;

            if(GlobalPlayerTurn)
            {
                level.timer.Start();
            }
            else
            {
                level.timer.Pause();
            }

            for(auto const &unit : level.combatants)
                unit->Activate();
            cursor.PlaceAt(level.Leader());
            viewportCol = 0; // TODO: Now this is a lie!
            viewportRow = 0;
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;

            ai.clearQueue();
            handler.clearQueue();
        }
        //////////////// ABOVE TO BE EXTRICATED //////////////////

// ============================= render =========================================
        Render(level.map, cursor, gameMenu, unitMenu);

        // IMGUI
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if(GlobalPlayerTurn)
			RenderUI(&ui, cursor, level.map, level.timer);
#if DEV_MODE
        if(GlobalEditorMode)
            EditorPass(&units, &level);
#endif
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(GlobalRenderer);

    } // End Game Loop

    Close();
    return 0;
}
