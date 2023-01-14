// Author: Alex Hartford
// Program: Emblem
// File: Constants

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ========================= constants =====================================
// meta
#define DEV_MODE 1

// low level
#define MS_PER_FRAME 16.666
#define JOYSTICK_DEAD_ZONE 8000
// TODO: Create a separate system for keyboard inputs
//       OR Decide on one input system or the other.
#define JOYSTICK_COOLDOWN_TIME 6 // Frames

// rendering
#define VIEWPORT_WIDTH 15
#define VIEWPORT_HEIGHT 10
#define TILE_SIZE 60
#define SCREEN_WIDTH 1120
#define SCREEN_HEIGHT 800

#define PORTRAIT_SIZE 400
#define SPRITE_SIZE 32
#define ATLAS_TILE_SIZE 16

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50
#define CONVERSATION_WRAP 840

// animation
#define ANIMATION_SPEED 10
#define AI_ACTION_SPEED 15

#define DATA_PATH "../data/"
#define CONVERSATIONS_PATH "../data/conversations/"
#define SPRITES_PATH "../assets/sprites/"
#define THUMBS_PATH "../assets/portraits/thumbs/"
#define FULLS_PATH "../assets/portraits/fulls/"
#define TILESETS_PATH "../assets/tilesets/"
#define INITIAL_LEVEL "l0.txt"
#define INITIAL_UNITS "units.txt"

#define DEFAULT_PORTRAIT "ph.png"
#define DEFAULT_SHEET "thief.png"

// gameplay
#define LEADER_ID 0

#define LEVEL_TIME 99

#define FLOOR_TILE {FLOOR, 1, 0, nullptr, {14, 1}}
#define WALL_TILE {WALL, 99, 0, nullptr, {6, 22}}
#define FOREST_TILE {FOREST, 2, 40, nullptr, {0, 6}}
#define DESERT_TILE {DESERT, 3, 0, nullptr, {18, 29}}
#define OBJECTIVE_TILE {OBJECTIVE, 1, 0, nullptr, {31, 0}}

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

    ATTACK_TARGETING,

    HEAL_TARGETING,

    PREVIEW_ATTACK,
    PREVIEW_HEALING,

    GAME_MENU,
    GAME_MENU_OUTLOOK,
    GAME_MENU_OPTIONS,

    UNIT_MENU_ROOT,
    UNIT_INFO,

    ENEMY_INFO,
    ENEMY_RANGE,

    LEVEL_MENU,

    CONVERSATION,

    NO_OP,

    GAME_OVER,
};

enum AIState
{
    FINDING_NEXT,
    SELECTED,

    PLAYER_TURN
};

enum quadrant
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

// ============================ color palette ==================================
// Tile overlays
const SDL_Color moveColor =     {100, 100, 180, 100};
const SDL_Color pathColor =     {150, 150, 150, 100};
const SDL_Color aiMoveColor =   {150, 0, 0, 100};
const SDL_Color attackColor =   {250, 0, 0, 100};
const SDL_Color healColor =     {0, 255, 0, 100};

const SDL_Color exhaustedMod =  {50, 0, 50, 0};
const SDL_Color readyMod =      {255, 255, 255, 0};

const SDL_Color editorColor =   {255, 255, 0, 100};

// UI
const SDL_Color backgroundColor =   {165, 179, 201, 255};
const SDL_Color uiColor =           {255, 224, 137, 255};
const SDL_Color outlineColor =      {85, 70, 31, 255};
const SDL_Color uiAltColor =        {60, 100, 30, 255};
const SDL_Color enemyColor =        {150, 60, 30, 255};
const SDL_Color uiTextColor =       {0, 0, 0, 255};
const SDL_Color uiSelectorColor =   {119, 100, 55, 100};
const SDL_Color black =             {0, 0, 0, 255};

// Palette Colors
const SDL_Color red =       {205, 35, 67, 255};
const SDL_Color darkRed =   {94, 1, 18, 255};
const SDL_Color green =     {101, 204, 55, 255};
const SDL_Color darkGreen = {21, 85, 51, 255};
const SDL_Color blue =      {42, 101, 205, 255};
const SDL_Color darkBlue =  {22, 50, 125, 255};

// Combat
const SDL_Color allyHealthBarColor = {10, 10, 200, 255};
const SDL_Color allyHealthBarLosingColor = {10, 10, 70, 255};

const SDL_Color enemyHealthBarColor = {200, 10, 10, 255};
const SDL_Color enemyHealthBarLosingColor = {70, 10, 10, 255};

const SDL_Color healthBarLostColor = {10, 10, 10, 255};

#endif
