// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

/*
    TODO
	MVP
		Basic AI
        Level Transitions

		Three main Units
		Three levels
		Enemy range rendering


    NICE
		Items
			Equipping
			Trading
			Different Ranges / Action types
		Items and Trading Menus
		Animation
			Combat Scene
			Placing Unit moves them to that square (A-star)
			Generate Arrow while selecting?
		Figure out fps mystery
		Decide whether cancelling a selection should move your cursor back or not.


    BACKLOG
		Music (MiniAudio)
		Conversations
		Smooth Interaction Syntax (Just click on enemy to attack them)
		Switch to GameController API
		Show Tile details and basic unit overview when hovering.
		Tiles have properties 
			(That remain together and don't take four calls to set up.)
		Redo Trade/Attack/Heal Stuff? Inheritance?
 */


// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <iostream>
#include <fstream>
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


// ========================= constants =====================================

// low level
#define TIME_STEP 33.33333
#define JOYSTICK_DEAD_ZONE 8000

// rendering
#define TILE_SIZE 100
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 900

#define MENU_WIDTH 240
#define MENU_ROW_HEIGHT 50

// backend
#define MAP_SIZE 8

// animation
#define CURSOR_MOVE_SPEED 2



// ============================= globals ===================================
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static TTF_Font *GlobalFont = nullptr;
static bool GlobalRunning = false;
static bool GlobalPlayerTurn = true;
static bool GlobalTurnStart = false;
static bool GlobalGamepadMode = false;
static bool GlobalGuiMode = false;



// ============================ structs ====================================
struct InputState
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
};


struct Texture
{
    SDL_Texture *sdlTexture;
    string path;
    int width;
    int height;

    Texture(SDL_Texture *sdlTexture_in, string path_in, int width_in, int height_in)
    {
        this->sdlTexture = sdlTexture_in;
        this->width = width_in;
        this->height = height_in;
        this->path = path_in;
    }
};

struct SpriteSheet
{
    shared_ptr<Texture> texture;
    int size    = 32;
    int tracks  = 0;
    int frames  = 0;
    int track   = 0;
    int frame   = 0;
    int speed   = 1; // inverse. 1 is faster than 10.
    int counter = 0;

    int colPixelOffset = 0;
    int rowPixelOffset = 0;

    // called each frame
    void Update()
    {
        counter++;
        if(counter % speed == 0)
        {
            int newFrame = frame + 1;
            if(newFrame >= frames)
            {
                this->frame = 0;
            }
            else
            {
                this->frame = newFrame;
            }
        }
    }

    // switches the sprite to the next animation track
    void ChangeTrack(int track_in)
    {
        if(track_in >= tracks || track_in < 0)
        {
            assert(!"SpriteSheet | ChangeTrack | Invalid Animation Track!\n");
        }
        this->track = track_in;
        this->frame = 0;
    }

    SpriteSheet(shared_ptr<Texture> texture_in, int size_in, int speed_in)
    {
        this->texture = texture_in;
        this->size = size_in;
        this->speed = speed_in;

        this->tracks = texture_in->height / size_in;
        this->frames = texture_in->width / size_in;
    }
};

struct Unit
{
    string name;
    int id;
    bool isAlly;
	int col = 0;
	int row = 0;
    bool isExhausted = false;
    bool shouldDie = false;
    int mov;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int healing = 0;
    int minRange;
    int maxRange;
    int accuracy;
    shared_ptr<SpriteSheet> sheet = nullptr;

    void Update()
    {
        sheet->Update();
    }

    Unit(string name_in, shared_ptr<Texture> texture_in, 
         int id_in, bool isAlly_in, int mov_in,
         int hp_in, int maxHp_in,
         int minRange_in, int maxRange_in,
         int attack_in, int defense_in, int accuracy_in)
    {
        this->name = name_in;
        this->sheet = make_shared<SpriteSheet>(texture_in, 32, 6);
        this->id = id_in;
        this->isAlly = isAlly_in;
        this->mov = mov_in;
        this->hp = hp_in;
        this->maxHp = maxHp_in;
        this->minRange = minRange_in;
        this->maxRange = maxRange_in;
        this->attack = attack_in;
        this->defense = defense_in;
        this->accuracy = accuracy_in;
    }

	~Unit()
	{
		printf("Unit Destroyed!\n");
	}
};


// map stuff
enum TileTypes
{
    FLOOR,
    WALL
};
struct Tile
{
    int type = 0;
    int penalty = 1;
    bool occupied = false;
    std::shared_ptr<Unit> occupant = nullptr;
};
struct Tilemap
{
    Tile tiles[MAP_SIZE][MAP_SIZE] = {};
    vector<pair<int, int>> accessible;
    vector<pair<int, int>> interactible;
};

struct Level
{
	Tilemap map;
	vector<shared_ptr<Unit>> allies;
	vector<shared_ptr<Unit>> enemies;

	void RemoveDeadUnits()
	{
		vector<pair<int, int>> tiles;

		for(shared_ptr<Unit> u : allies)
		{
			if(u->shouldDie)
			{
				tiles.push_back(pair<int, int>(u->col, u->row)); 
			}
		}
		for(shared_ptr<Unit> u : enemies)
		{
			if(u->shouldDie)
			{
				tiles.push_back(pair<int, int>(u->col, u->row));
			}
		}

		allies.erase(remove_if(allies.begin(), allies.end(), [](shared_ptr<Unit> u) { return u->shouldDie; }), allies.end());
		enemies.erase(remove_if(enemies.begin(), enemies.end(), [](shared_ptr<Unit> u) { return u->shouldDie; }), enemies.end());

		for(pair<int, int> tile : tiles)
		{
			map.tiles[tile.first][tile.second].occupant = nullptr;
			map.tiles[tile.first][tile.second].occupied = false;
		}
	}
};

struct Cursor
{
    int col = 1;
    int row = 1;
    std::shared_ptr<Unit> selected = nullptr;
    std::shared_ptr<Unit> targeted = nullptr;
    int selectedCol = -1; // Where the cursor was before placing a unit
    int selectedRow = -1;
    int sourceCol = -1; // Where the cursor was before choosing a target
    int sourceRow = -1;

    shared_ptr<SpriteSheet> sheet;

    Cursor(shared_ptr<Texture> texture_in)
    {
        this->sheet = make_shared<SpriteSheet>(texture_in, 32, 6);
    }

    void Update()
    {
        sheet->Update();
    }
};


// menu stuff
#include "load.h"
struct Menu
{
    u8 rows;
    u8 current;

    vector<unique_ptr<Texture>> optionTextTextures;

    Menu(u8 rows_in, u8 current_in, vector<string> options_in)
    : rows(rows_in),
      current(current_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};
struct UnitInfo
{
    u8 rows;

    vector<unique_ptr<Texture>> infoTextTextures;

    UnitInfo(u8 rows_in, vector<string> info_in)
    : rows(rows_in)
    {
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }

    void UpdateTextTextures(vector<string> info_in)
    {
        infoTextTextures.clear();
        int newRows = 0;
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
    }
};
struct CombatInfo
{
    u8 rows;

    vector<unique_ptr<Texture>> sourceTextTextures;
    vector<unique_ptr<Texture>> targetTextTextures;

    CombatInfo(u8 rows_in, vector<string> sourceInfo_in, vector<string> targetInfo_in)
    : rows(rows_in)
    {
        for(string s : sourceInfo_in)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }

        for(string s : targetInfo_in)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        } 
    }

    void UpdateTextTextures(vector<string> sourceInfo_in, vector<string> targetInfo_in)
    {
        assert(sourceInfo_in.size() == targetInfo_in.size());
        sourceTextTextures.clear();
        targetTextTextures.clear();
        int newRows = 0;
        for(string s : sourceInfo_in)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
        for(string s : targetInfo_in)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};


// ===================== function signatures ==============================
bool Initialize();
void Close();
void HandleEvents(InputState *input);

#include "fight.h"
#include "grid.h"
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
    vector<shared_ptr<Unit>> units = LoadCharacters("../data/units.txt");
    shared_ptr<Level> level = LoadLevel("../data/l1.txt", units);
    Cursor cursor(LoadTextureImage("../assets/sprites/cursor.png"));

	// initial actor state
    InputState input;
    InputHandler handler(&cursor, level->map);
	AI ai;

    // Initialize Menus
    Menu gameMenu(3, 0, {"Outlook", "Options", "End Turn"});
    Menu unitMenu(6, 0, {"Info", "Items", "Attack", "Heal", "Trade", "Wait"});
    UnitInfo unitInfo(1, {"Placeholder"});
    CombatInfo combatInfo(1, {"Placeholder"}, {"Placeholder"});

	// debug messages
    unique_ptr<Texture> debugMessageOne = LoadTextureText("placeholder1", {250, 0, 0, 255});
    unique_ptr<Texture> debugMessageTwo = LoadTextureText("placeholder2", {0, 100, 0, 255});
    unique_ptr<Texture> debugMessageThree = LoadTextureText("placeholder3", {0, 0, 250, 255});

	// frame timer
    real32 TargetMillisecondsPerFrame = 16.666f;
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime = 0;
    u64 frameNumber = 0;
    real32 ElapsedMS = 0.0f;

// ========================= game loop =========================================
    GlobalRunning = true;
    while(GlobalRunning)
    {
        HandleEvents(&input);

// ====================== command phase ========================================
		if(GlobalTurnStart)
		{
			for(shared_ptr<Unit> unit : level->enemies)
			{
				unit->isExhausted = false;
			}
			for(shared_ptr<Unit> unit : level->allies)
			{
				unit->isExhausted = false;
			}
			GlobalTurnStart = false;
			ai.commandQueue = {};
		}

		if(GlobalPlayerTurn)
		{
			handler.Update(&input);
			handler.UpdateCommands(&cursor, &level->map,
								   &gameMenu, &unitMenu, 
								   &unitInfo, &combatInfo);
        }
        else
        {
			if(ai.shouldPlan)
			{
				ai.Plan(&cursor, &level->map);
			}
			if(!(frameNumber % 10))
			{
				ai.Update();
			}
		}



// ========================= update phase =======================================
        cursor.Update();

        for(shared_ptr<Unit> unit : level->allies)
        {
            unit->Update();
        }
        for(shared_ptr<Unit> unit : level->enemies)
        {
            unit->Update();
        }
		level->RemoveDeadUnits();


// ============================= render =========================================
        Render(level->map, cursor, gameMenu, unitMenu, unitInfo, combatInfo,
               *debugMessageOne, *debugMessageTwo, *debugMessageThree);


// =========================== v debug messages v ============================================
        endTime = SDL_GetPerformanceCounter();
        ElapsedMS = ((endTime - startTime) / (real32)SDL_GetPerformanceFrequency()) * 1000.0f;

        // Debug Messages
        char buffer[256];

        sprintf(buffer, "Mode: %d", GlobalInterfaceState);
        debugMessageOne = LoadTextureText(string(buffer), {250, 0, 0, 255});

        sprintf(buffer, "Tile <%d, %d> | Type: %d, Occupied: %d, Occupant: %p",
               cursor.col, cursor.row, level->map.tiles[cursor.col][cursor.row].type,
               level->map.tiles[cursor.col][cursor.row].occupied,
               (void *)level->map.tiles[cursor.col][cursor.row].occupant.get());
        debugMessageTwo = LoadTextureText(string(buffer), {0, 100, 0, 255});

        sprintf(buffer, "MS: %.02f, FPS: %d", ElapsedMS, (int)(1.0f / ElapsedMS * 1000.0f));
        debugMessageThree = LoadTextureText(string(buffer), {0, 0, 250, 255});

        if(ElapsedMS < TargetMillisecondsPerFrame)
        {
            SDL_Delay((int)(TargetMillisecondsPerFrame - ElapsedMS));
        }
        startTime = SDL_GetPerformanceCounter();
        frameNumber++;
    }
    Close();
    return 0;
}


// ================================== SDL Functions ================================================

// Initializes SDL and Dear ImGui
bool
Initialize()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == 0)
    {
        SDL_WindowFlags windowFlags = (SDL_WindowFlags)(
                                                        SDL_WINDOW_RESIZABLE | 
                                                        //SDL_WINDOW_ALLOW_HIGHDPI |
                                                        SDL_WINDOW_SHOWN
                                                        );
        GlobalWindow = SDL_CreateWindow("Emblem",
                                        700, // Placement for debugging
                                        200,
                                        SCREEN_WIDTH,
                                        SCREEN_HEIGHT,
                                        windowFlags);
        if(GlobalWindow)
        {
            GlobalRenderer = SDL_CreateRenderer(GlobalWindow, -1, 
                    SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
            if(GlobalRenderer)
            {
                if(TTF_Init() != -1)
                {
                    GlobalFont = TTF_OpenFont("../assets/fonts/verdanab.ttf", 28);
                    if(GlobalFont)
                    {
                        int imgFlags = IMG_INIT_PNG;
                        if(IMG_Init(imgFlags) & imgFlags)
                        {
                            // Setup Dear ImGui context
                            //IMGUI_CHECKVERSION();
                            //ImGui::CreateContext();
                            //ImGuiIO& io = ImGui::GetIO(); (void)io;

                            //io.Fonts->AddFontFromFileTTF("../assets/fonts/verdanab.ttf", 10.0f);

                            // Setup Dear ImGui style
                            //ImGui::StyleColorsDark();

                            // Setup Platform/Renderer backends
                            //ImGui_ImplSDL2_InitForSDLRenderer(GlobalWindow, GlobalRenderer);
                            //ImGui_ImplSDLRenderer_Init(GlobalRenderer);

                            return true;
                        }
                        assert(!"ERROR: IMG\n");
                    }
                    else
                    {
                        assert(!"ERROR: Font Opening\n");
                    }
                }
                else
                {
                    assert(!"ERROR: TTF\n");
                }
            }
            else
            {
                assert(!"ERROR: Renderer\n");
            }
        }
        else
        {
            assert(!"ERROR: Window\n");
        }
    }
    else
    {
        assert(!"ERROR: SDL Init\n");
    }
    return false;
}


// Frees up allocated memory.
void Close()
{
    //SDL_DestroyTexture(ALL TEXTURES);

    //Close game controller
    //SDL_JoystickClose(gGameController);

    //ImGui_ImplSDLRenderer_Shutdown();
    //ImGui_ImplSDL2_Shutdown();
    //ImGui::DestroyContext();

    TTF_CloseFont(GlobalFont);
    SDL_DestroyRenderer(GlobalRenderer);
    SDL_DestroyWindow(GlobalWindow);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


// Handles all SDL Events, including key presses.
void
HandleEvents(InputState *input)
{
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_TAB)
        {
            GlobalGuiMode = !GlobalGuiMode;
        }
        if(Event.type == SDL_QUIT || Event.key.keysym.sym == SDLK_ESCAPE)
        {
            GlobalRunning = false;
        }
        if(GlobalGuiMode)
        {
            //ImGui_ImplSDL2_ProcessEvent(&Event);
        }
        else
        {
            if(Event.type == SDL_KEYDOWN)
            {
                switch(Event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                    {
                        input->a = true;
                    } break;

                    case SDLK_LSHIFT:
                    {
                        input->b = true;
                    } break;

                    case SDLK_w:
                    {
                        input->up = true;
                    } break;

                    case SDLK_s:
                    {
                        input->down = true;
                    } break;

                    case SDLK_a:
                    {
                        input->left = true;
                    } break;

                    case SDLK_d:
                    {
                        input->right = true;
                    } break;

                    default:
                    {
                    } break;
                }
            }
            else if(Event.type == SDL_KEYUP)
            {
                switch(Event.key.keysym.sym)
                {
                    case SDLK_w:
                    {
                        input->up = false;
                    } break;

                    case SDLK_s:
                    {
                        input->down = false;
                    } break;

                    case SDLK_a:
                    {
                        input->left = false;
                    } break;

                    case SDLK_d:
                    {
                        input->right = false;
                    } break;

                    case SDLK_SPACE:
                    {
                        input->a = false;
                    } break;

                    case SDLK_LSHIFT:
                    {
                        input->b = false;
                    } break;

                    default:
                    {
                    } break;
                }
            }
            else if(Event.type == SDL_JOYAXISMOTION)
            {
                if(Event.jaxis.which == 0)
                {
                    if(Event.jaxis.axis == 0)
                    {
                        if(Event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            input->left = true;
                        }
                        else if(Event.jaxis.value > JOYSTICK_DEAD_ZONE)
                        {
                            input->right = true;
                        }
                        else
                        {
                            input->left = false;
                            input->right = false;
                        }
                    }

                    if(Event.jaxis.axis == 1)
                    {
                        if(Event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            input->up = true;
                        }
                        else if(Event.jaxis.value > JOYSTICK_DEAD_ZONE)
                        {
                            input->down = true;
                        }
                        else
                        {
                            input->up = false;
                            input->down = false;
                        }
                    }
                }
            }
            else if(Event.type == SDL_JOYBUTTONDOWN || Event.type == SDL_JOYBUTTONUP)
            {
                if(Event.jbutton.button == 0)
                {
                    if(Event.type == SDL_JOYBUTTONDOWN)
                    {
                        input->a = true;
                    }
                    else if(Event.type == SDL_JOYBUTTONUP)
                    {
                        input->a = false;
                    }
                }
                if(Event.jbutton.button == 1)
                {
                    if(Event.type == SDL_JOYBUTTONDOWN)
                    {
                        input->b = true;
                    }
                    else if(Event.type == SDL_JOYBUTTONUP)
                    {
                        input->b = false;
                    }
                }
            }
            else if(Event.type == SDL_JOYDEVICEADDED)
            {
                printf("Gamepad connected!\n");
                GlobalGamepadMode = true;
            }
            else if(Event.type == SDL_JOYDEVICEREMOVED)
            {
                printf("Gamepad removed!\n");
                GlobalGamepadMode = false;
            }
        }
    }
}
