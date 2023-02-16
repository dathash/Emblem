// Author: Alex Hartford
// Program: Emblem
// File: Constants

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ========================= constants =====================================
// meta
#define DEV_MODE 1

// low level
#define JOYSTICK_DEAD_ZONE 8000
// TODO: Create a separate system for keyboard inputs
//       OR Decide on one input system or the other.
#define JOYSTICK_COOLDOWN_TIME 5 // Frames

// defaults
#define DEFAULT_MUSIC_VOLUME 0.0f
#define DEFAULT_SFX_VOLUME 0.05f

// rendering
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define TILE_SIZE 60
#define SCREEN_WIDTH 1120
#define SCREEN_HEIGHT 800
#define X_OFFSET 400
#define Y_OFFSET 10

#define PORTRAIT_SIZE 600
#define SPRITE_SIZE 32
#define ATLAS_TILE_SIZE 16

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50
#define CONV_MENU_WIDTH 400

// animation
#define ANIMATION_SPEED 10
#define AI_ACTION_SPEED 10

// startup
#define INITIAL_LEVEL "l0.txt"
#define INITIAL_UNITS "units.tsv"
#define INITIAL_EQUIPS "equipments.tsv"

// data
#define DATA_PATH "../data/"
#define LEVELS_PATH "../data/levels/"

// assets
#define MUSIC_PATH "../assets/music/"
#define SFX_PATH "../assets/sfx/"
#define SPRITES_PATH "../assets/sprites/"
#define FULLS_PATH "../assets/portraits/fulls/"
#define TILESETS_PATH "../assets/tilesets/"

#define DEFAULT_PORTRAIT "ph.png"
#define DEFAULT_SHEET "thief.png"

// gameplay
#define LEADER_ID hash<string>{}("Lucina")

#define EXP_FOR_COMBAT 1
#define EXP_FOR_KILL 2

#define FLOOR_TILE {FLOOR, nullptr, {0, 0}}
#define WALL_TILE {WALL, nullptr, {1, 0}}
#define FOREST_TILE {FOREST, nullptr, {2, 0}}
#define SWAMP_TILE {SWAMP, nullptr, {5, 1}}
#define FORT_TILE {FORT, nullptr, {4, 0}}
#define GOAL_TILE {GOAL, nullptr, {5, 0}}
#define VILLAGE_TILE {VILLAGE, nullptr, {1, 1}}
#define CHEST_TILE {CHEST, nullptr, {0, 1}}

enum InterfaceState
{
    NO_OP,
    TITLE_SCREEN,
    GAME_OVER,

    GAME_MENU,
    GAME_MENU_OPTIONS,

    ENEMY_RANGE,

    NEUTRAL_OVER_GROUND,
    NEUTRAL_OVER_ENEMY,
    NEUTRAL_OVER_UNIT,
    NEUTRAL_OVER_DEACTIVATED_UNIT,

    SELECTED,

    ATTACK_THINKING,
    ATTACK_TARGETING,
    ATTACK_RESOLUTION,
};

// AI
enum AIState
{
    AI_NO_OP,
    AI_FINDING_NEXT,
    AI_SELECTED,

    AI_ATTACK_RESOLUTION,
};

enum Direction
{
    DIR_NONE,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
};

// MAP
enum TileType
{
    FLOOR,
    WALL,
    FOREST,
    SWAMP,
    GOAL,
    FORT,
    VILLAGE,
    CHEST,
};

// ============================ color palette ==================================
// Tile overlays
const SDL_Color moveColor =         {100, 100, 180, 100};
const SDL_Color pathColor =         {100, 250, 250, 100};
const SDL_Color aiMoveColor =       {150, 0, 0, 100};
const SDL_Color attackColor =       {250, 0, 0, 100};
const SDL_Color healColor =         {0, 255, 0, 100};
const SDL_Color clearColor =        {0, 0, 0, 0};

const SDL_Color exhaustedMod =      {100,   0, 100, 0};
const SDL_Color readyMod =          {255, 255, 255, 0};

const SDL_Color editorColor =       {255, 255, 0, 100};

// UI
const SDL_Color backgroundColor =   {165, 179, 201, 255};
const SDL_Color uiColor =           {255, 224, 145, 255};
const SDL_Color uiDarkColor =       {138, 110, 39, 255};
const SDL_Color uiTitleColor =      {120, 100, 50, 255};
const SDL_Color outlineColor =      {85, 70, 31, 255};
const SDL_Color uiAltColor =        {60, 100, 30, 255};
const SDL_Color enemyColor =        {150, 60, 30, 255};
const SDL_Color uiTextColor =       {0, 0, 0, 255};
const SDL_Color uiSelectorColor =   {119, 100, 55, 100};

// Palette Colors
const SDL_Color black =             {0,   0,   0,   255};
const SDL_Color white =             {255, 255, 255, 255};
const SDL_Color red =               {205, 35,  67,  255};
const SDL_Color darkRed =           {94,  1,   18,  255};
const SDL_Color green =             {101, 204, 55,  255};
const SDL_Color darkGreen =         {21, 85, 51, 255};
const SDL_Color blue =              {42, 101, 205, 255};
const SDL_Color darkBlue =          {22, 50, 125, 255};
const SDL_Color purple =            {100, 20, 125, 255};


const SDL_Color yellow =            {200, 200, 0, 255};
const SDL_Color darkGray =          {100, 100, 100, 255};
const SDL_Color accentBlue =        {106, 150, 250, 255};
const SDL_Color cerulean =          {0, 128, 255, 255};

#endif
