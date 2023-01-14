// Author: Alex Hartford
// Program: Emblem
// File: Main

/* Style Guide
 * GlobalVariable;
 * local_variable;
 *
 * struct Name
 * {
 *     field_name;
 * };
 *
 * type
 * FunctionName()
 * {
 *     ...;
 *     return;
 * }
 *
 * everything else, just go nuts.
 */

// ========================== includes =====================================
// Library includes (Linked in on the machine)
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

// Local external files (in /ext)
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

// C++ stdlib
#include <iostream>
using namespace std;

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
//
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

// Note: I don't have anything against global state in this program, since I
// don't intend to multi-thread it. However, I keep it to a minimum and
// consolidate it as much as I can, so that it doesn't carry a lot of mental
// overhead.

static bool GlobalRunning = false;
static bool GlobalEditorMode = false;
static bool GlobalPlayerTurn = true;
static bool GlobalLevelTransition = false;

// Transitory
static bool GlobalNextLevel = false;
static bool GlobalRestart = false;
static bool GlobalTurnStart = true;

static unsigned int GlobalInterfaceState;
static unsigned int GlobalAIState;

////////////////////////////////

// TODO: Refactor these to only be where they need to be.
//       Pretty sure none of them have to be globals.
static int viewportCol = 0;
static int viewportRow = 0;
static int GlobalCurrentID = 0;

// ================================= my includes ===============================
// NOTE: This is a unity build. This is all that the game includes.
#include "constants.h"
#include "utils.h"
#include "state.h"
#include "structs.h"
// TODO: Figure out how to fix this forward declaration
static Timer GlobalLevelTimer = Timer(LEVEL_TIME);
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

    vector<string> levels = {DATA_PATH + string("test.txt"), DATA_PATH + string("l1.txt"),
                             DATA_PATH + string("l2.txt"), DATA_PATH + string("l3.txt"),
                             DATA_PATH + string("l4.txt"), DATA_PATH + string("l5.txt"),
                             DATA_PATH + string("l6.txt"), DATA_PATH + string("l7.txt")
							 };
    int level_index = 0;
    Level level = LoadLevel(levels[level_index], units);
    Cursor cursor(Spritesheet(LoadTextureImage(SPRITES_PATH, string("cursor.png")), 
		32, ANIMATION_SPEED));

	UI_State ui = {};

    Menu game_menu({"Outlook", "Options", "End Turn"});
    Menu unit_menu({"Wait"});
    Menu level_menu({"Next", "Redo", "Conv"});

    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    Conversation conversation = LoadConversation(CONVERSATIONS_PATH, "test.txt", 
                                *level.combatants[0], *level.combatants[1]);

    GlobalInterfaceState = NO_OP;
    GlobalAIState = PLAYER_TURN;

	// Play Music
    //ma_engine_play_sound(&GlobalMusicEngine, "../assets/audio/r4.wav", NULL);

    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        HandleEvents(&input, gamepad);

        // Update
        if(!GlobalEditorMode)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level.map,
                                   &game_menu, &unit_menu, &level_menu,
                                   &conversation);

            ai.Update(&cursor, &level.map);

            cursor.Update();
            ui.Update();
            level.Update();
            if(GlobalLevelTimer.Update())
                RestartLevel();

            for(const unique_ptr<Unit> &unit : level.combatants)
                unit->Update();
        }

        // Resolve State
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
            GlobalLevelTimer = Timer(LEVEL_TIME);
            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
        }

        if(GlobalTurnStart)
        {
            GlobalTurnStart = false;

            if(GlobalPlayerTurn)
                GlobalLevelTimer.Start();
            else
                GlobalLevelTimer.Pause();

            for(auto const &unit : level.combatants)
                unit->Activate();
            cursor.PlaceAt(level.Leader());
            viewportCol = 0;
            viewportRow = 0;
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;

            ai.clearQueue();
            handler.clearQueue();
        }
        //////////////// ABOVE TO BE EXTRICATED //////////////////

        // Render
        Render(level.map, cursor, game_menu, unit_menu, level_menu, conversation);

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

    } // End Game Loop

    Close();
    return 0;
}
