// Author: Alex Hartford
// Program: Emblem
// File: Main

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

// Transitory
static bool GlobalNextLevel = false;
static bool GlobalTurnStart = false;

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
#include "audio.h" // NOTE: Includes GlobalMusic and GlobalSfx, GlobalSong
#include "event.h" // NOTE: Includes a GlobalEvents queue.
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
        SDL_assert(!"Initialization Failed\n");

    // controller init
    SDL_Joystick *gamepad = NULL;
    if(SDL_NumJoysticks() > 0)
    {
        gamepad = SDL_JoystickOpen(0);
        SDL_assert(gamepad);
    }

    GlobalMusic.sounds =
    {
        new Sound("spiny.wav", MUSIC),
        new Sound("r4.wav", MUSIC),
        new Sound("qc.wav", MUSIC),
        new Sound("6.wav", MUSIC),
        new Sound("13.wav", MUSIC),
        new Sound("21.wav", MUSIC),
        new Sound("tripod.wav", MUSIC),
        new Sound("fs.wav", MUSIC),
        new Sound("spacelion.wav", MUSIC),
        new Sound("fire.wav", MUSIC),
        new Sound("boat.wav", MUSIC),
        new Sound("forest.wav", MUSIC),
        new Sound("town.wav", MUSIC),
        new Sound("gerudo.wav", MUSIC),
        new Sound("village.wav", MUSIC),
        new Sound("hamduche.wav", MUSIC),
        new Sound("wind.wav", MUSIC),
        new Sound("chrono.wav", MUSIC),
        new Sound("title.wav", MUSIC)
    };

    GlobalSfx.sounds =
    {
        new Sound("mov.wav", SFX),
        new Sound("crit.wav", SFX),
        new Sound("heal.wav", SFX),
        new Sound("hit1.wav", SFX),
        new Sound("hit2.wav", SFX),
        new Sound("hit3.wav", SFX),
        new Sound("magic.wav", SFX),
        new Sound("miss.wav", SFX),
        new Sound("pickup.wav", SFX),
        new Sound("place.wav", SFX),
        new Sound("powerup.wav", SFX),
        new Sound("ranged.wav", SFX),
        new Sound("sel1.wav", SFX),
        new Sound("sel2.wav", SFX),
        new Sound("sel3.wav", SFX),
        new Sound("start.wav", SFX),
        new Sound("dance.wav", SFX)
    };

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
    Menu conversation_menu({"Return"});

    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    Fight fight;

    GlobalInterfaceState = TITLE_SCREEN;
    GlobalAIState = PLAYER_TURN;

    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        GlobalHandleEvents();
        HandleEvents(&input, gamepad);

        // Update
        if(!GlobalEditorMode)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level, units,
                                   &game_menu, &unit_menu, 
                                   &level_menu, &conversation_menu,
                                   &fight);

            ai.Update(&cursor, &level.map, &fight);

            cursor.Update(&level.map);
            fight.Update();
            level.Update();
            ui.Update();

            for(const shared_ptr<Unit> &unit : level.combatants)
                unit->Update();
        }

        // Resolve State
        //////////////// TO BE EXTRICATED //////////////////
        if(GlobalNextLevel)
        {
            GlobalNextLevel = false;

            level_index = (level_index + 1 < levels.size()) ? level_index + 1 : 0;

            level.song->Stop();
            level = LoadLevel(DATA_PATH + levels[level_index], units);
            level.conversations.prelude.song->Start();

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
                        unit->TickBuff();
                }
            }

            for(auto const &unit : level.combatants)
                unit->Activate();

            cursor.PlaceAt(level.Leader());
            SetViewport(cursor.pos, level.map.width, level.map.height);

            // TODO: This is pretty bad. Just work out how the logic should actually go.
            if(GlobalInterfaceState != PRELUDE)
                GlobalInterfaceState = NEUTRAL_OVER_UNIT;

            ai.clearQueue();
            handler.clearQueue();
        }

        // TODO : Definitely get rid of this.
        if(GlobalInterfaceState == GAME_OVER)
        {
            GlobalPlayerTurn = true;
            ai.clearQueue();
        }
        //////////////// ABOVE TO BE EXTRICATED //////////////////

        // Render
        Render(level.map, cursor, game_menu, unit_menu, level_menu, conversation_menu,
               level.conversations, fight);

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

    // This needs to be in this function due to scope restrictions.
    // TODO: Learn more about heap allocation, scope weirdness, double frees, etc.
    for(Sound *sound : GlobalMusic.sounds)
        delete sound;
    for(Sound *sound : GlobalSfx.sounds)
        delete sound;

    Close();
    return 0;
}
