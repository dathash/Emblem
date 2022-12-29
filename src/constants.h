// Author: Alex Hartford
// Program: Emblem
// File: Constants
// Date: July 2022

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ========================= constants =====================================
// meta
#define EDITOR 1

// low level
#define MS_PER_FRAME 16.666
#define JOYSTICK_DEAD_ZONE 8000
#define JOYSTICK_COOLDOWN_TIME 4 // Frames

// rendering
#define VIEWPORT_SIZE 8
#define TILE_SIZE 60
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define PORTRAIT_SIZE 300
#define SPRITE_SIZE 32
#define ATLAS_TILE_SIZE 16

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50

#define HEALTH_TICK_HEIGHT 40
#define HEALTH_TICK_WIDTH 15

// animation
#define ANIMATION_SPEED 10

#define DATA_PATH "../data/"
#define SPRITES_PATH "../assets/sprites/"
#define PORTRAITS_PATH "../assets/portraits/"
#define TILESETS_PATH "../assets/tilesets/"
#define INITIAL_LEVEL "l1.txt"
#define INITIAL_UNITS "units.txt"

#define DEFAULT_PORTRAIT "ph.png"
#define DEFAULT_SHEET "warrior.png"


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

enum AIState
{
    FINDING_NEXT,
    SELECTED,
    FOUND_NEW_POSITION,
    PLACED,

    ENEMY_TURN
};
// =============================== tiles =======================================
// TODO put global tile types here, instead of macro solution in structs.h.

// ============================ color palette ==================================
// Tile overlays
const SDL_Color moveColor = {100, 100, 180, 100};
const SDL_Color pathColor = {150, 150, 150, 100};
const SDL_Color aiMoveColor = {150, 0, 0, 100};
const SDL_Color attackColor = {250, 0, 0, 100};
const SDL_Color healColor = {0, 255, 0, 100};

const SDL_Color exhaustedMod = {50, 0, 50, 0};
const SDL_Color readyMod = {255, 255, 255, 0};

const SDL_Color editorColor = {255, 255, 0, 100};

// UI
const SDL_Color backgroundColor = {165, 179, 201, 255};
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
