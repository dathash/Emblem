// Author: Alex Hartford
// Program: Emblem
// File: Globals and Constants
// Date: July 2022

#ifndef GLOBALS_H
#define GLOBALS_H

// ========================= constants =====================================

// low level
#define MS_PER_FRAME 16.666
#define JOYSTICK_DEAD_ZONE 8000
#define JOYSTICK_COOLDOWN_TIME 4 // Frames

// rendering
#define TILE_SIZE 70
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50

#define HEALTH_TICK_HEIGHT 40
#define HEALTH_TICK_WIDTH 15

// animation
#define ANIMATION_SPEED 10


// ============================= globals ===================================

enum InterfaceState
{
    NEUTRAL_OVER_GROUND,
    NEUTRAL_OVER_ENEMY,
    NEUTRAL_OVER_UNIT,
    NEUTRAL_OVER_DEACTIVATED_UNIT,

    SELECTED_OVER_GROUND,
    SELECTED_OVER_INACCESSIBLE,
    SELECTED_OVER_ALLY,
    SELECTED_OVER_ENEMY,

    ATTACK_TARGETING_OVER_UNTARGETABLE,
    ATTACK_TARGETING_OVER_TARGET,

    HEALING_TARGETING_OVER_UNTARGETABLE,
    HEALING_TARGETING_OVER_TARGET,

    PREVIEW_ATTACK,
    PREVIEW_HEALING,

    GAME_MENU_ROOT,
    GAME_MENU_OUTLOOK,
    GAME_MENU_OPTIONS,

    UNIT_MENU_ROOT,
    UNIT_INFO,

    ENEMY_INFO,

    NO_OP,
};

static InterfaceState GlobalInterfaceState = NEUTRAL_OVER_GROUND;

enum AIState
{
    FINDING_NEXT,
    SELECTED,
    FOUND_NEW_POSITION,
    PLACED,

    ENEMY_TURN
};

static AIState GlobalAIState = ENEMY_TURN;


static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;
static ma_engine GlobalMusicEngine;
static bool GlobalRunning = false;
static u64 GlobalFrameNumber = 0;
static int GlobalLevelNumber = 0;
static bool GlobalNextLevel = false;
static int GlobalTotalLevels = 2;

static bool GlobalResolvingAction = false;

static bool GlobalPlayerTurn = true;
static bool GlobalTurnStart = false;
static bool GlobalGamepadMode = false;
static bool GlobalGuiMode = false;


// ============================ color palette ==============================
// Tiles
const SDL_Color floorColor = {255, 255, 255, 255};
const SDL_Color wallColor = {50, 50, 50, 255};

// Overlays
const SDL_Color moveColor = {0, 150, 0, 100};
const SDL_Color aiMoveColor = {150, 0, 0, 100};
const SDL_Color attackColor = {250, 0, 0, 100};
const SDL_Color healColor = {0, 255, 0, 100};
const SDL_Color exhaustedMod = {50, 0, 50, 0};
const SDL_Color readyMod = {255, 255, 255, 0};

// UI
const SDL_Color backgroundColor = {200, 200, 10, 255};
const SDL_Color uiColor = {60, 60, 150, 255};
const SDL_Color outlineColor = {10, 10, 10, 255};
const SDL_Color uiAltColor = {60, 100, 30, 255};
const SDL_Color enemyColor = {150, 60, 30, 255};
const SDL_Color uiTextColor = {255, 255, 255, 255};
const SDL_Color uiSelectorColor = {50, 50, 50, 100};

// Combat
const SDL_Color allyHealthBarColor = {10, 10, 200, 255};
const SDL_Color allyHealthBarLosingColor = {10, 10, 70, 255};

const SDL_Color enemyHealthBarColor = {200, 10, 10, 255};
const SDL_Color enemyHealthBarLosingColor = {70, 10, 10, 255};

const SDL_Color healthBarLostColor = {10, 10, 10, 255};


#endif
