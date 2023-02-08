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

#define EPSILON 0.00001

// defaults
#define DEFAULT_MUSIC_VOLUME 0.0f
#define DEFAULT_SFX_VOLUME 0.05f

// rendering
#define VIEWPORT_WIDTH 15
#define VIEWPORT_HEIGHT 10
#define TILE_SIZE 60
#define SCREEN_WIDTH 1120
#define SCREEN_HEIGHT 800

#define PORTRAIT_SIZE 600
#define SPRITE_SIZE 32
#define ATLAS_TILE_SIZE 16

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50
#define CONV_MENU_WIDTH 400
#define CONVERSATION_WRAP 840

// animation
#define ANIMATION_SPEED 10
#define AI_ACTION_SPEED 10

// startup
#define INITIAL_LEVEL "l0.txt"
#define INITIAL_UNITS "units.tsv"

// data
#define DATA_PATH "../data/"
#define CONVERSATIONS_PATH "../data/conversations/"
#define VILLAGES_PATH "../data/conversations/villages/"
#define PRELUDES_PATH "../data/conversations/preludes/"

// assets
#define MUSIC_PATH "../assets/music/"
#define SFX_PATH "../assets/sfx/"
#define SPRITES_PATH "../assets/sprites/"
#define THUMBS_PATH "../assets/portraits/thumbs/"
#define FULLS_PATH "../assets/portraits/fulls/"
#define TILESETS_PATH "../assets/tilesets/"

#define DEFAULT_PORTRAIT "ph.png"
#define DEFAULT_SHEET "thief.png"

// gameplay
#define LEADER_ID hash<string>{}("Lucina")

#define DOUBLE_RATIO 2    // How much a unit must beat another by to double them.
#define CRIT_MULTIPLIER 2 // The multiplier on damage.

#define EXP_FOR_COMBAT 5
#define EXP_FOR_HEALING 10
#define EXP_FOR_DANCE 10
#define EXP_FOR_BUFF 10

#define EXP_FOR_VILLAGE_SAVED 40

#define FLOOR_TILE {FLOOR, 1, 0, 0, nullptr, {14, 1}}
#define WALL_TILE {WALL, 99, 0, 0, nullptr, {6, 22}}
#define FOREST_TILE {FOREST, 2, 20, 0, nullptr, {0, 6}}
#define SWAMP_TILE {SWAMP, 3, 0, 0, nullptr, {18, 29}}
#define FORT_TILE {FORT, 1, 10, 4, nullptr, {1, 0}}
#define GOAL_TILE {GOAL, 1, 0, 2, nullptr, {31, 0}}
#define SPAWN_TILE {SPAWN, 1, 0, 0, nullptr, {14, 1}}
#define VILLAGE_TILE {VILLAGE, 1, 0, 0, nullptr, {31, 25}}
#define CHEST_TILE {CHEST, 1, 0, 0, nullptr, {0, 1}}

enum InterfaceState
{
    NEUTRAL_OVER_GROUND, // 0
    NEUTRAL_OVER_ENEMY, // 1
    NEUTRAL_OVER_UNIT, // 2
    NEUTRAL_OVER_DEACTIVATED_UNIT, // 3

    SELECTED_OVER_GROUND, // 4
    SELECTED_OVER_INACCESSIBLE, // 5
    SELECTED_OVER_ALLY, // 6
    SELECTED_OVER_ENEMY, // 7

    ATTACK_TARGETING, // 8
    ABILITY_TARGETING, // 9
    TALK_TARGETING, // 10

    PREVIEW_ATTACK, // 11
    PREVIEW_ABILITY, // 12

    GAME_MENU, // 13
    GAME_MENU_OUTLOOK, // 14
    GAME_MENU_OPTIONS, // 15

    ANIMATING_UNIT_MOVEMENT, // 16

    UNIT_MENU_ROOT, // 17
    UNIT_INFO, // 18

    ENEMY_INFO, // 19
    ENEMY_RANGE, // 20

    LEVEL_MENU, // 21

    CONVERSATION_MENU, // 22
    CONVERSATION, // 23
    BATTLE_CONVERSATION, // 24
    VILLAGE_CONVERSATION, // 25
    PRELUDE, // 26

    PLAYER_FIGHT, // 27
    RESOLVING_EXPERIENCE, // 28

    NO_OP, // 29

    TITLE_SCREEN, // 30
    GAME_OVER, // 31
};


// AI
enum AIState
{
    FINDING_NEXT, // 0
    SELECTED, // 1

    AI_RESOLVING_EXPERIENCE, // 2

    AI_FIGHT, // 3

    PLAYER_TURN, // 4
};

enum AIBehavior
{
    NO_BEHAVIOR,
    PURSUE,
    PURSUE_AFTER_1,
    PURSUE_AFTER_2,
    PURSUE_AFTER_3,
    BOSS,
    BOSS_THEN_MOVE,
    ATTACK_IN_RANGE,
    ATTACK_IN_TWO,
    FLEE,
    TREASURE_THEN_FLEE,
};

// UI
enum quadrant
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

// MAP
enum TileType
{
    FLOOR,
    WALL,
    FOREST,
    SWAMP,
    GOAL,
    SPAWN,
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

const SDL_Color exhaustedMod =      {50,    0,  50, 0};
const SDL_Color buffAtkMod =        {255,   0,   0, 0};
const SDL_Color buffDefMod =        {0,   255, 255, 0};
const SDL_Color buffAptMod =        {255, 150, 255, 0};
const SDL_Color buffSpdMod =        {255, 255,   0, 0};
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

const SDL_Color yellow =            {200, 200, 50, 255};
const SDL_Color darkGray =          {100, 100, 100, 255};
const SDL_Color accentBlue =        {106, 150, 250, 255};
const SDL_Color cerulean =          {0, 128, 255, 255};

#endif
