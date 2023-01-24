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
#include <queue>
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

//// STATE //////////////////////
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
static int viewportCol = 0;
static int viewportRow = 0;

// ================================= my includes ===============================
// NOTE: This is a unity build. This is all that the game includes.
#include "constants.h"
#include "utils.h"
#include "audio.h" // NOTE: Includes GlobalMusic and GlobalSfx.
#include "event.h" // NOTE: Includes a GlobalEvents queue.
#include "state.h"
#include "structs.h"
#include "animation.h"
#include "cursor.h"
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
    vector<shared_ptr<Unit>> units = LoadUnits(DATA_PATH + string(INITIAL_UNITS));

    vector<string> levels = {string("l0.txt"), string("l1.txt"),
                             string("l2.txt"), string("l3.txt"),
                             string("l4.txt"), string("l5.txt"),
                             string("l6.txt"), string("l7.txt")
							 };
    int level_index = 0;
    Level level = LoadLevel(DATA_PATH + levels[level_index], units);
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
                                                 *units[0], *units[1]);

    Fight fight;

    GlobalInterfaceState = NO_OP;
    GlobalAIState = PLAYER_TURN;

	// Play Music
    PlayMusic("spiny.wav");

    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        GlobalHandleEvents(); // TODO: Better naming scheme, or make this not global.
        HandleEvents(&input, gamepad);

        // Update
        if(!GlobalEditorMode)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level.map,
                                   &game_menu, &unit_menu, &level_menu,
                                   &conversation, &fight);

            ai.Update(&cursor, &level.map, &fight);

            cursor.Update();
            fight.Update();
            level.Update();
            ui.Update();

            for(const shared_ptr<Unit> &unit : level.combatants)
                unit->Update();
        }

        // Resolve State
        //////////////// TO BE EXTRICATED //////////////////
        if(GlobalRestart)
        {
            GlobalRestart = false;
            level = LoadLevel(DATA_PATH + levels[level_index], units);
            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
        }

        if(GlobalNextLevel)
        {
            GlobalNextLevel = false;
            level_index = (level_index + 1 < levels.size()) ? level_index + 1 : 0;
            vector<shared_ptr<Unit>> party = {};

            // Persistence (Naive)
            for(shared_ptr<Unit> unit : level.combatants)
            {
                if(unit->is_ally && 
                   unit->ID() != LEADER_ID)
                    party.push_back(unit);
            }

            level = LoadLevel(DATA_PATH + levels[level_index], units);

            // Persistence (Naive)
            for(shared_ptr<Unit> unit : party)
            {
                level.AddCombatant(unit, level.map.GetNextSpawnLocation());
            }

            GlobalPlayerTurn = true;
            GlobalTurnStart = true;
        }

        if(GlobalTurnStart)
        {
            GlobalTurnStart = false;


            if(GlobalPlayerTurn)
            {
                for(auto const &unit : level.combatants)
                {
                    if(unit->buff)
                    {
                        unit->TickBuff();
                    }
                }
            }

            for(auto const &unit : level.combatants)
            {
                unit->Activate();
            }
            cursor.PlaceAt(level.Leader());
            SetViewport(cursor.pos, level.map.width, level.map.height);
            GlobalInterfaceState = NEUTRAL_OVER_UNIT;

            ai.clearQueue();
            handler.clearQueue();
        }

        if(GlobalInterfaceState == GAME_OVER)
        {
            GlobalPlayerTurn = true;
            ai.clearQueue();
        }
        //////////////// ABOVE TO BE EXTRICATED //////////////////

        // Render
        Render(level.map, cursor, game_menu, unit_menu, level_menu, 
               conversation, fight);

        // IMGUI
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
        RenderUI(&ui, cursor, level.map, fight);

#if DEV_MODE
        if(GlobalEditorMode)
            EditorPass(&units, &level, levels);
#endif

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(GlobalRenderer);

    } // End Game Loop

    Close();
    return 0;
}
