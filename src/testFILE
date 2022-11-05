// Author: Alex Hartford
// Program: Emblem
// File: Tests
// Date: July 2022

// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <miniaudio.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;
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


// If parameter is not true, test fails
#define IS_TRUE(x) { if (!(x)) cout << __FUNCTION__ << " failed on line " << __LINE__ << "\n"; }
#define EQUALS(x, y) { if (!(x == y)) cout << __FUNCTION__ << " failed on line " << __LINE__ << "\n"; }

// ================================= test grid functions ==========================================
void TestGridFunctions()
{
    // IsValidBoundsPosition
    IS_TRUE(IsValidBoundsPosition(3, 3, 1, 1));
    IS_TRUE(IsValidBoundsPosition(3, 3, 2, 2));
    IS_TRUE(IsValidBoundsPosition(3, 3, 0, 0));
    IS_TRUE(!IsValidBoundsPosition(0, 0, 0, 0));
    IS_TRUE(!IsValidBoundsPosition(0, 0, 1, 1));
    IS_TRUE(!IsValidBoundsPosition(8, 8, -1, -1));

    // WithinViewport

    // MoveViewport

    // VectorHasElement
    IS_TRUE(!VectorHasElement(
                {1, 3}, {}));
    IS_TRUE(VectorHasElement(
                {1, 3}, {{1, 3}}));
    IS_TRUE(!VectorHasElement(
                {4, 3}, 
                {{1, 3}, {2, 4}}));

    // AccessibleFrom

    // InteractibleFrom

    // FindNearest

    // FindClosestAccessibleTile

    // FindVictim

    // Distance
}

// ================================= test fight functions ==========================================
void TestFightFunctions()
{
    // CalculateDamage
    EQUALS(0, CalculateDamage(0, 0));
    EQUALS(2, CalculateDamage(4, 2));
    EQUALS(0, CalculateDamage(4, 4));
    EQUALS(96, CalculateDamage(100, 4));
    EQUALS(0, CalculateDamage(0, 4));

    // SimulateCombat

    // SimulateHealing
}

// ================================= test spritesheet ==========================================
void TestSpriteSheetUpdate()
{
	SpriteSheet sheet(LoadTextureImage("../assets/testing/test.png"), 32, ANIMATION_SPEED);
    EQUALS(12, sheet.tracks);
    EQUALS(4, sheet.frames);
	EQUALS(ANIMATION_SPEED, sheet.speed);
    EQUALS(0, sheet.counter);
	sheet.Update();
    EQUALS(1, sheet.counter);
	// TODO More testing to be done here
}

void TestSpriteSheetChangeTrack()
{
	SpriteSheet sheet(LoadTextureImage("../assets/testing/test.png"), 32, ANIMATION_SPEED);
	EQUALS(0, sheet.track);
	sheet.ChangeTrack(3);
	EQUALS(3, sheet.track);
	EQUALS(0, sheet.frame);
	sheet.ChangeTrack(0);
	EQUALS(0, sheet.track);
	EQUALS(0, sheet.frame);
}

void TestSpriteSheet()
{
	TestSpriteSheetUpdate();	
	TestSpriteSheetChangeTrack();	
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if(!Initialize())
        assert(!"Initialization Failed\n");

    TestGridFunctions();
    TestFightFunctions();
	TestSpriteSheet();
	/* TODO: */
	//TestUnit();
	//TestCursor();
	//TestMenus();
	//TestTilemap();
	//TestLevelLoader();
	//TestCommands();
	//TestAI();

    Close();
    return 0;
}
