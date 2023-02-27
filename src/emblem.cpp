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

// My Globals
static bool GlobalRunning = false;
static bool GlobalEditorMode = false;
static bool GlobalDebug = true;

static bool GlobalPaused = false;
static bool GlobalStep = false;
static int GlobalSpeedMod = 1;
static bool GlobalSpawning = true;

#include "constants.h"
// State
static Phase GlobalPhase = PHASE_PLAYER;
static InterfaceState GlobalInterfaceState;
static AIState GlobalAIState;

// ================================= my includes ===============================
// NOTE: This is a unity build. This file is the only compilation unit, and it
// pulls all of the following includes in at compile time.

#include "utils.h"
#include "event.h" // NOTE: Includes Global Event handler
#include "animation.h"
#include "audio.h" // NOTE: Includes Global Audio engine and Sound groups, as well as GlobalMusic and GlobalSfx.
#include "equip.h"
#include "structs.h"
static Player GlobalPlayer;

#include "cursor.h"
#include "state.h"
#include "vfx.h"
#include "load.h"
#include "init.h"
#include "input.h"
#include "grid.h"
#include "fight.h"
#include "resolution.h"
#include "command.h"
#include "ai.h"
#include "ui.h"
#include "render.h"
#include "editor.h"
#include "debug.h"
#include "test.h"

// ================================= the main ==================================
int main(int argc, char *argv[])
{
#if DEV_MODE
    RunTests();
#endif

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
        new Sound("boost.wav", SFX),
        new Sound("chaching.wav", SFX),
        new Sound("clang.wav", SFX),
        new Sound("click.wav", SFX),
        new Sound("crit.wav", SFX),
        new Sound("dance.wav", SFX),
        new Sound("denied.wav", SFX),
        new Sound("experience.wav", SFX),
        new Sound("heal.wav", SFX),
        new Sound("hinges.wav", SFX),
        new Sound("hit1.wav", SFX),
        new Sound("hit2.wav", SFX),
        new Sound("hit3.wav", SFX),
        new Sound("hooves.wav", SFX),
        new Sound("horsey1.wav", SFX),
        new Sound("horsey2.wav", SFX),
        new Sound("levelup.wav", SFX),
        new Sound("magic.wav", SFX),
        new Sound("miss.wav", SFX),
        new Sound("mission_complete.wav", SFX),
        new Sound("mission_start.wav", SFX),
        new Sound("nock.wav", SFX),
        new Sound("pickup.wav", SFX),
        new Sound("pitch.wav", SFX),
        new Sound("place.wav", SFX),
        new Sound("powerup.wav", SFX),
        new Sound("ranged.wav", SFX),
        new Sound("rubble.wav", SFX),
        new Sound("sel1.wav", SFX),
        new Sound("sel2.wav", SFX),
        new Sound("sel3.wav", SFX),
        new Sound("start.wav", SFX),
        new Sound("turn.wav", SFX),
        new Sound("whoosh1.wav", SFX),
        new Sound("whoosh2.wav", SFX),
        new Sound("whoosh3.wav", SFX),
        new Sound("whoosh4.wav", SFX)
    };

// ================================== load =================================
    vector<shared_ptr<Equip>> equipments = LoadEquips(DATA_PATH + string(INITIAL_EQUIPS));
    vector<shared_ptr<Unit>> units = LoadUnits(DATA_PATH + string(INITIAL_UNITS), equipments);
    vector<shared_ptr<Unit>> party = {};

    vector<string> levels = {string("l0.txt"), string("l1.txt"),
                             string("l2.txt"), string("l3.txt"),
                             string("l4.txt"), string("l5.txt"),
                             string("l6.txt"), string("l7.txt"),
							 };
    int level_index = 0;
    Level level = LoadLevel(levels[level_index], units, party);

    Cursor cursor(Spritesheet(LoadTextureImage(SPRITES_PATH, string("cursor.png")), 
		                      32, ANIMATION_SPEED));

	UI_State ui = {};

    Menu game_menu({"Options", "End Turn"});

    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    Resolution resolution;

    GlobalInterfaceState = TITLE_SCREEN;
    GlobalAIState = AI_NO_OP;

    unsigned long long frame = 0;

    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        ++frame;
        if(frame > 900000000000000) // Just in case we overflow!
            frame = 0;

        HandleEvents(&input, gamepad);

        // Update
        if(!GlobalEditorMode && 
           (!GlobalPaused || GlobalStep) &&
           !(frame % GlobalSpeedMod))
        {
            GlobalStep = false;

            switch(GlobalPhase)
            {
            case PHASE_AI:
            {
                ai.Update(&cursor, &level, &resolution);
            } break;
            case PHASE_PLAYER:
            {
                handler.Update(&input);
                handler.UpdateCommands(&cursor, &level, units, party, &game_menu);
            } break;
            case PHASE_RESOLUTION:
            {
                resolution.Update(&(level.map));
            } break;
            case PHASE_SPAWNING:
            {
                if(GlobalSpawning)
                    level.SpawnPhase();
                GoToPlayerPhase(&level, &cursor);
            } break;
            }

            cursor.Update(&level.map);

            // NOTE: Must be in this order.
            resolution.RemoveDeadUnits();
            level.RemoveDeadUnits();

            level.Update();

            EventSystemUpdate();

            for(const shared_ptr<Unit> &unit : level.combatants)
                unit->Update();

            for(Sound *sound : GlobalMusic.sounds)
                sound->Update();
        }

        /*
        // Resolve State
        if(level.next_level)
        {
            level.next_level = false;
            level_index = (level_index + 1 < levels.size()) ? level_index + 1 : 0;

            party = {};
            for(shared_ptr<Unit> unit : level.combatants)
            {
                if(unit->IsAlly())
                {
                    unit->health = unit->max_health;
                    unit->is_exhausted = false;
                    party.push_back(unit);
                }
            }
            level.song->Stop();
            level = LoadLevel(levels[level_index], units, party);

            GoToAIPhase();
        }
        */

        // Render
        Render(level.map, level.spawner.rising, cursor, game_menu, resolution);

        // IMGUI
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
        RenderUI(&ui, cursor, level, GlobalPlayer);

#if DEV_MODE
        if(GlobalEditorMode)
            EditorPass(&equipments, &units, party, &level, levels, &resolution);
        if(GlobalDebug)
            DebugUI();
#endif

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(GlobalRenderer);
    } // End Game Loop

    for(Sound *sound : GlobalMusic.sounds)
        delete sound;
    for(Sound *sound : GlobalSfx.sounds)
        delete sound;

    Close();
    return 0;
}
