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
// * Next Level

// Note: I don't have anything against global state in this program, since I
// don't intend to multi-thread it. However, I keep it to a minimum and
// consolidate it as much as I can, so that it doesn't carry a lot of mental
// overhead.

static bool GlobalRunning = false;
static bool GlobalEditorMode = false;
static bool GlobalPlayerTurn = true;

////////////////////////////////

// TODO: Refactor these to only be where they need to be.
static int viewportCol = 0;
static int viewportRow = 0;

////////////////////////////////

// EVENTS
// TODO: Fix these circular dependencies.
enum EventType
{
    START_GAME_EVENT,
    NEXT_LEVEL_EVENT,
    END_PLAYER_TURN_EVENT,
    END_AI_TURN_EVENT,
    FADE_DONE_EVENT,
    MOVE_CURSOR_EVENT,
    MOVE_MENU_EVENT,
    SELECT_MENU_OPTION_EVENT,
    NEXT_SENTENCE_EVENT,
    PICK_UP_UNIT_EVENT,
    PLACE_UNIT_EVENT,
    UNIT_INFO_EVENT,
    ATTACK_HIT_EVENT,
    ATTACK_CRIT_EVENT,
    ATTACK_MISS_EVENT,
    ATTACK_RANGED_EVENT,
    BUFF_EVENT,
    HEAL_EVENT,
    DANCE_EVENT,
    EXPERIENCE_EVENT,
    EXPERIENCE_DONE_EVENT,
    ADVANCEMENT_EVENT,
    LEVEL_BOOST_EVENT,
};

struct Unit;
struct Event
{
    EventType type;
    Unit *unit = nullptr;
    int integer = 0;
    float number = 0.0f;

    Event(EventType type_in)
    : type(type_in)
    {}

    Event(EventType type_in,
          Unit *unit_in,
          int integer_in = 0,
          float number_in = 0.0f)
    : type(type_in),
      unit(unit_in),
      integer(integer_in),
      number(number_in)
    {}
};

static queue<Event> GlobalEvents;

void
EmitEvent(Event event)
{
    GlobalEvents.push(event);
}

// ================================= my includes ===============================
// NOTE: This is a unity build. This is all that the game includes.
#include "constants.h"
static InterfaceState GlobalInterfaceState;
static AIState GlobalAIState;

#include "utils.h"
#include "animation.h"
#include "audio.h" // NOTE: Includes GlobalMusic and GlobalSfx, GlobalSong
#include "structs.h"
#include "vfx.h"
#include "event.h" // NOTE: Includes a GlobalEvents queue.
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
        new Sound("miss01.wav", SFX),
        new Sound("miss04.wav", SFX),
        new Sound("pickup.wav", SFX),
        new Sound("place.wav", SFX),
        new Sound("powerup.wav", SFX),
        new Sound("ranged.wav", SFX),
        new Sound("sel1.wav", SFX),
        new Sound("sel2.wav", SFX),
        new Sound("sel3.wav", SFX),
        new Sound("start.wav", SFX),
        new Sound("click.wav", SFX),
        new Sound("minor_click.wav", SFX),
        new Sound("turn.wav", SFX),
        new Sound("experience.wav", SFX),
        new Sound("levelup.wav", SFX),
        new Sound("boost.wav", SFX),
        new Sound("dance.wav", SFX)
    };

// ================================== load =================================
    vector<shared_ptr<Unit>> units = LoadUnits(DATA_PATH + string(INITIAL_UNITS));
    vector<shared_ptr<Unit>> party = {};


    vector<string> levels = {string("l0.txt"), string("l1.txt"),
                             string("l2.txt"), string("l3.txt"),
                             string("l4.txt"), string("l5.txt"),
                             string("l6.txt"), string("l7.txt")
							 };
    int level_index = 0;
    Level level = LoadLevel(DATA_PATH + levels[level_index], units, party);

    Cursor cursor(Spritesheet(LoadTextureImage(SPRITES_PATH, string("cursor.png")), 
		32, ANIMATION_SPEED));

	UI_State ui = {};

    Menu game_menu({"Outlook", "Options", "End Turn"});
    Menu unit_menu({"Wait"});
    Menu level_menu({"Next", "Redo", "Conv"});
    Menu conversation_menu({"Return"});

    Fade level_fade = {"t", "t"};
    Fade turn_fade = {"Player Turn", "Enemy Turn"};
    Parcel parcel;
    Advancement advancement;

    InputState input = {};
    InputHandler handler(&cursor, level.map);
    AI ai;

    Fight fight;

    GlobalInterfaceState = TITLE_SCREEN;
    GlobalAIState = AI_PLAYER_TURN;

    GlobalRunning = true;
// ========================= game loop =========================================
    while(GlobalRunning)
    {
        HandleEvents(&input, gamepad);

        // Update
        if(!GlobalEditorMode)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level, units, party,
                                   &game_menu, &unit_menu, 
                                   &level_menu, &conversation_menu,
                                   &fight);

            ai.Update(&cursor, &level, &fight);

            cursor.Update(&level.map);
            fight.Update();
            parcel.Update();
            advancement.Update();
            level.Update();
            level_fade.Update();
            turn_fade.Update();

            for(const shared_ptr<Unit> &unit : level.combatants)
                unit->Update();

            for(Sound *sound : GlobalMusic.sounds)
                sound->Update();

            if(GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT &&
               ((level.objective == OBJECTIVE_ROUT &&
                 level.GetNumberOf(false) == 0)
                 ||
                (level.objective == OBJECTIVE_BOSS && 
                 level.IsBossDead())))
            {
                cout << "IN CONDITION\n";
                level.song->FadeOut();
                GlobalInterfaceState = LEVEL_MENU;
            }
        }

        // Resolve State
        if(level.next_level)
        {
            level.next_level = false;
            level_index = (level_index + 1 < levels.size()) ? level_index + 1 : 0;

            level = level.LoadNextLevel(levels[level_index], units, &party);
        }
        if(level.CheckNextTurn())
        {
            cursor.PlaceAt(level.Leader());
            SetViewport(cursor.pos, level.map.width, level.map.height);

            ai.clearQueue();
            handler.clearQueue();
        }

        GlobalHandleEvents(&level_fade, &turn_fade, &parcel, &advancement);

        // Render
        Render(level.map, cursor, game_menu, unit_menu, level_menu, conversation_menu,
               level.conversations, fight, level_fade, turn_fade, advancement);

        // IMGUI
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
        RenderUI(&ui, cursor, level, fight, parcel, advancement);

#if DEV_MODE
        if(GlobalEditorMode)
            EditorPass(&units, party, &level, levels);
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
