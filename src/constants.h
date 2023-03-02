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
#define TILE_SIZE 80
#define SCREEN_WIDTH 1120
#define SCREEN_HEIGHT 800
#define X_OFFSET 200
#define Y_OFFSET 10

#define PORTRAIT_SIZE 600
#define SPRITE_SIZE 32
#define SPRITE_SCALE 1.2f
#define ATLAS_TILE_SIZE 16

#define MENU_WIDTH 250
#define MENU_ROW_HEIGHT 50
#define CONV_MENU_WIDTH 400

// animation
#define ANIMATION_SPEED 10
#define AI_ACTION_SPEED 15

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
#define LEADER_ID hash<string>{}("Guy")

#define POINTS_FOR_PLAYER_UNIT 5
#define POINTS_FOR_BUILDING 5
#define POINTS_FOR_AI_UNIT -2

enum Phase
{
    PHASE_SPAWNING,
    PHASE_AI,
    PHASE_PLAYER,
    PHASE_RESOLUTION,
};
string 
GetPhaseString(Phase phase)
{
    switch(phase)
    {
        case PHASE_AI:          return "AI";
        case PHASE_PLAYER:      return "Player";
        case PHASE_RESOLUTION:  return "Resolution";
        case PHASE_SPAWNING:    return "Spawning";
	}
}

enum InterfaceState
{
    NO_OP,
    TITLE_SCREEN,
    GAME_OVER,
    VICTORY,

    //OPTIONS,

    QUEUE,
    UNDO,

    ENEMY_RANGE,

    NEUTRAL,
    SELECTED,

    ATTACK_THINKING,
    ATTACK_TARGETING,
    ATTACK_RESOLUTION,

    WARP,
};
string 
GetInterfaceString(InterfaceState state)
{
    switch(state)
    {
    case NO_OP:             return "No-Op";
    case TITLE_SCREEN:      return "Title screen";
    case GAME_OVER:         return "Game over";
    case VICTORY:           return "Victory";
    case QUEUE:             return "Queue";
    case UNDO:              return "Undo";
    case ENEMY_RANGE:       return "Enemy range";
    case NEUTRAL:           return "Neutral";
    case SELECTED:          return "Selected";
    case ATTACK_THINKING:   return "Attack thinking";
    case ATTACK_TARGETING:  return "Attack targeting";
    case ATTACK_RESOLUTION: return "Attack resolution";
    case WARP:              return "Warp";
	}
}

// AI
enum AIState
{
    AI_NO_OP,
    AI_FINDING_NEXT,
    AI_SELECTED,

    AI_ATTACK_RESOLUTION,
};

string 
GetAIString(AIState state)
{
    switch (state)
    {
    case AI_NO_OP:                  return "No-op";
    case AI_FINDING_NEXT:           return "Finding Next";
    case AI_SELECTED:               return "Selected";
    case AI_ATTACK_RESOLUTION:      return "Attack Resolution";
    }
}

// ============================ color palette ==================================
// Palette Colors
const SDL_Color lightred =          {155, 26,  10,  255};
const SDL_Color red =               {85, 15,   10,  255};
const SDL_Color darkred =           {48, 15,   10,  255};

const SDL_Color lightgreen =        {165, 140, 39,  255};
const SDL_Color green =             {57, 87, 28,  255};
const SDL_Color darkgreen =         {31, 36, 10, 255};

const SDL_Color lightorange =       {239, 105, 47,  255};
const SDL_Color orange =            {239, 58, 12,  255};
const SDL_Color darkorange =        {54, 23, 12, 255};

const SDL_Color lightblue =         {60, 159, 156, 255};
const SDL_Color blue =              {39, 100, 104, 255};
const SDL_Color darkblue =          {24, 63, 57, 255};

const SDL_Color yellow =            {239, 172, 40, 255};

const SDL_Color lightbrown =        {171, 92, 28, 255};
const SDL_Color brown =             {114, 65, 19, 255};
const SDL_Color darkbrown =         {69, 35, 13, 255};

const SDL_Color lightbeige =        {239, 183, 117, 255};
const SDL_Color beige =             {165, 98, 67, 255};
const SDL_Color darkbeige =         {119, 52, 33, 255};

const SDL_Color lightgray =         {146, 126, 106, 255};
const SDL_Color gray =              {104, 76, 60, 255};
const SDL_Color darkgray =          {57, 42, 28, 255};

const SDL_Color white =             {239, 216, 161, 255};
const SDL_Color black =             {42, 29, 13, 255};

const int OverlayAlphaMod = 128;

#endif
