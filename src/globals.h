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
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;
static bool GlobalRunning = false;
static u64 GlobalFrameNumber = 0;
static int GlobalLevelNumber = 0;
static bool GlobalNextLevel = false;
static int GlobalTotalLevels = 2;

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

// UI
const SDL_Color backgroundColor = {200, 200, 0, 255};
const SDL_Color uiColor = {60, 60, 150, 255};
const SDL_Color outlineColor = {0, 0, 0, 255};
const SDL_Color uiAltColor = {60, 100, 30, 255};
const SDL_Color enemyColor = {150, 60, 30, 255};
const SDL_Color uiTextColor = {255, 255, 255, 255};
const SDL_Color uiSelectorColor = {50, 50, 50, 100};

// Combat
const SDL_Color healthBarColor = {0, 150, 0, 255};
const SDL_Color healthBarLostColor = {0, 70, 0, 255};




#endif
