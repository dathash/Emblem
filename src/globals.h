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

// animation
#define ANIMATION_SPEED 10


// ============================= globals ===================================
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;
static bool GlobalRunning = false;
static int GlobalLevelNumber = 0;
static bool GlobalNextLevel = false;
static int GlobalTotalLevels = 2;

static bool GlobalPlayerTurn = true;
static bool GlobalTurnStart = false;
static bool GlobalGamepadMode = false;
static bool GlobalGuiMode = false;

#endif
