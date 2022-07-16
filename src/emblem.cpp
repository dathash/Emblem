// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

/*
    TODO
    MVP
        Show Tile details and basic unit overview when hovering.
        Combat Scene animation

        Design
            Three main Units
            Three levels

        Better enemy ai
            A*
            Goes for "beneficial trades"
                Higher damage output than input.
                kills
            healing?
            Ranged Attacks
        
        BUGS
        Combat Preview is wrong.
        AI actions don't move the viewport.

    NICE
        Testing
        Making more Pure Functions where I can.
        Level Transitions/Win Screen
        Attacks locking onto first target.
        Cannot attack if no targets
        Statistics determine things
        Magic / Res
        Animation
            Placing Unit moves them to that square (A-star)
            Generate Arrow while selecting?
        Smooth Interaction Syntax (Just click on enemy to attack them)
        Draw Attack range squares


    BACKLOG
        Decide whether cancelling a selection should move your cursor back or not.
        Music (MiniAudio)
 */


// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <miniaudio.h>

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float real32;
typedef double real64;

using namespace std;

#include "globals.h"
#include "structs.h"
#include "init.h"
#include "grid.h"
#include "fight.h"
#include "command.h"
#include "ai.h"
#include "render.h"


// ================================== main =================================
int main(int argc, char *argv[])
{
    srand(time(NULL));

    if(!Initialize())
        assert(!"Initialization Failed\n");

    // controller init
    SDL_Joystick *gamePad = NULL;
    if(SDL_NumJoysticks() < 1)
    {
        printf("Keyboard Mode!\n");
    }
    else
    {
        GlobalGamepadMode = true;
        gamePad = SDL_JoystickOpen(0);
        assert(gamePad);
        printf("Joystick mode!\n");
    }

    // load data
    vector<unique_ptr<Unit>> units = LoadCharacters("../data/units.txt");
    vector<string> levels = {"../data/l1.txt", "../data/l2.txt"};
    Level level = LoadLevel(levels[GlobalLevelNumber], units);
    Cursor cursor(SpriteSheet(LoadTextureImage("../assets/sprites/cursor.png"), 32, ANIMATION_SPEED));

    // Initialize Menus
    Menu gameMenu(3, 0, {"Outlook", "Options", "End Turn"});
    Menu unitMenu(4, 0, {"Info", "Attack", "Heal", "Wait"});
    UnitInfo unitInfo(1, {"Placeholder"});
    TileInfo tileInfo(1, {"Placeholder"});
    CombatInfo combatInfo(1, {"Placeholder"}, {"Placeholder"});

    // initial actor state
    InputState input = {};
    InputHandler handler(&cursor, level.map, &tileInfo);
    AI ai;

    // frame timer
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime = 0;
    real32 elapsedMS = 0.0f;

// ========================= game loop =========================================
    GlobalRunning = true;
    while(GlobalRunning)
    {
        HandleEvents(&input);

// ====================== command phase ========================================
        if(GlobalTurnStart)
        {
            for(auto const &unit : level.enemies)
            {
                unit->isExhausted = false;
            }
            for(auto const &unit : level.allies)
            {
                unit->isExhausted = false;
            }
            cursor.col = 0;
            cursor.row = 0;
            cursor.viewportCol = 0;
            cursor.viewportRow = 0;
            GlobalInterfaceState = NEUTRAL_OVER_GROUND;

            GlobalTurnStart = false;
            ai.clearQueue();
            handler.clearQueue();
        }


        if(GlobalPlayerTurn)
        {
            handler.Update(&input);
            handler.UpdateCommands(&cursor, &level.map,
                                   &gameMenu, &unitMenu, 
                                   &unitInfo, &tileInfo,
                                   &combatInfo);
        }
        else
        {
            if(ai.shouldPlan)
            {
                ai.Plan(&cursor, &level.map);
            }
            if(!(GlobalFrameNumber % 10))
            {
                ai.Update();
            }
        }


// ========================= update phase =======================================
        cursor.Update();

        for(auto const &unit : level.allies)
        {
            unit->Update();
        }
        for(auto const &unit : level.enemies)
        {
            unit->Update();
        }

        // cleanup
        level.RemoveDeadUnits();

        if(GlobalNextLevel)
        {
            GlobalNextLevel = false;
            GlobalLevelNumber = (GlobalLevelNumber + 1 < GlobalTotalLevels) ? GlobalLevelNumber + 1 : 0;
            level = LoadLevel(levels[GlobalLevelNumber], units);
        }


// ============================= render =========================================
        Render(level.map, cursor, gameMenu, unitMenu, unitInfo, tileInfo, combatInfo);


// =========================== v debug messages v ============================================
        endTime = SDL_GetPerformanceCounter();
        elapsedMS = ((endTime - startTime) / (real32)SDL_GetPerformanceFrequency() * 1000.0f);

        if(elapsedMS < MS_PER_FRAME)
        {
            SDL_Delay((int)(MS_PER_FRAME - elapsedMS));
        }
        startTime = SDL_GetPerformanceCounter();
        GlobalFrameNumber++;
    }
    Close();
    return 0;
}
