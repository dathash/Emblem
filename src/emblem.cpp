// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

/*
    TODO
    User Interface
        Show Unit/Enemy details
        Show Tile details
    Switch to GameController API

    NEXT
    Combat
        Effects actually occur
    Menus
        Unit Actions Menu
        Game Menu
    Level System
        Loading
        Editing
        Saving
        Transitions
    Animation
        Synchronization
        Sprite Animation
        Combat Scene
        Key Repeat
    Music (MiniAudio)

    NICE
    Smooth Interaction Syntax (Just click on enemy to attack them)
    Leave characters that have moved and be able to use them to act later
    Replay!
    Hook in interactions
        Talk
        Items

    LATER
    Tiles have properties 
        (That remain together and don't rely on eachother)
    Turns
        Enemy Turn
        Basic AI
 */


// ========================= constants =====================================

// low level
#define JOYSTICK_DEAD_ZONE 8000

// rendering
#define TILE_SIZE 100
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 900

#define MENU_WIDTH 300
#define MENU_ROW_HEIGHT 100

// backend
#define MAP_SIZE 8


// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

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



// ============================ structs ====================================
struct InputState
{
    bool up;
    bool down;
    bool left;
    bool right;

    bool a;
    bool b;

    void PrintInputState()
    {
        printf("====== Input ======\n");
        printf("up  : %d\n", up);
        printf("down: %d\n", down);
        printf("left: %d\n", left);
        printf("rgt : %d\n", right);
        printf("a   : %d\n", a);
        printf("b   : %d\n", b);
    }
};

struct Texture
{
    SDL_Texture *sdlTexture;
    int width;
    int height;

    Texture(SDL_Texture *sdlTexture_in, int width_in, int height_in)
    {
        this->sdlTexture = sdlTexture_in;
        this->width = width_in;
        this->height = height_in;
        //printf("Texture Constructed!\n");
    }

    Texture()
    {
        //printf("Texture Constructed! (DEFAULT)\n");
    }

    ~Texture()
    {
        //printf("Texture Destructed!\n");
    }

    void PrintTextureState()
    {
        printf("====== Texture ======\n");
        printf("tex : %ld\n", (long int)sdlTexture);
        printf("wid : %d\n", width);
        printf("hgt : %d\n", height);
    }
};

struct SpriteSheet
{
    shared_ptr<Texture> texture;
    int size;
    int col;
    int row;

    SpriteSheet()
    {
        //printf("SpriteSheet Constructed! (Default)\n");
    }

    SpriteSheet(shared_ptr<Texture> texture_in, int size_in)
    {
        this->texture = texture_in;
        this->size = size_in;
        this->col = 0;
        this->row = 0;
        //printf("SpriteSheet Constructed!\n");
    }

    ~SpriteSheet()
    {
        //printf("SpriteSheet Destructed!\n");
    }

    void PrintSpriteState()
    {
        printf("====== Sprite ======\n");
        texture->PrintTextureState();
        printf("size: %d\n", size);
        printf("col : %d\n", col);
        printf("row : %d\n", row);
    }
};


struct Unit
{
    int id = 0;
    bool isAlly = false;
    bool isExhausted = false;
    int mov = 0;
    int hp = 10;
    int attack = 3;
    int heal = 2;
    int minRange = 1;
    int maxRange = 1;
    shared_ptr<SpriteSheet> sheet = nullptr;


    Unit(int id_in, bool isAlly_in, int mov_in, 
         int minRange_in, int maxRange_in, 
         shared_ptr<Texture> texture_in)
    {
        //printf("Unit Constructed!\n");
        this->id = id_in;
        this->isAlly = isAlly_in;
        this->mov = mov_in;
        this->minRange = minRange_in;
        this->maxRange = maxRange_in;
        this->sheet = make_shared<SpriteSheet>(texture_in, 32);
    }

    ~Unit()
    {
        //printf("Unit Destructed!\n");
    }
};


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
    shared_ptr<vector<pair<int, int>>> accessible = nullptr;
    shared_ptr<vector<pair<int, int>>> interactible = nullptr;
};

enum TargetMode
{
    NONE_TARGET,
    ATTACK_TARGET,
    HEAL_TARGET,
    TRADE_TARGET,
};

struct Cursor
{
    int col = 1;
    int row = 1;
    std::shared_ptr<Unit> selected = nullptr;
    std::shared_ptr<Unit> targeted = nullptr;
    int selectedCol = -1;
    int selectedRow = -1;
    int targeterCol = -1;
    int targeterRow = -1;
    TargetMode targetMode = NONE_TARGET;

    void PrintCursorState()
    {
        printf("====== Cursor ======\n");
        printf("Col : %d\n", col);
        printf("Row : %d\n", row);
        printf("Sel : %ld\n", (long int)selected.get());
    }
};

struct Menu
{
    u8 rows = 4;
    u8 current = 0;
    // TODO: Should a menu contain Command *'s?
};

#include "grid.h"
#include "command.h"

// ============================= globals ===================================
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static bool GlobalRunning = false;
static bool GlobalGamepadMode = false;
static bool GlobalGuiMode = false;
static TTF_Font *GlobalFont = nullptr;


#include "render.h"

// ===================== function signatures ==============================
shared_ptr<Texture> LoadTextureImage(std::string path);
unique_ptr<Texture> LoadTextureText(std::string text, SDL_Color color);

bool Initialize();
void Close();
void HandleEvents(InputState *input);



// ================================== main =================================
int main(int argc, char *argv[])
{
    if(!Initialize())
        assert(!"Initialization Failed\n");

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

    InputState input = {};
    InputHandler handler;
    queue<shared_ptr<Command>> commandQueue = {};

    real32 TargetMillisecondsPerFrame = 16.666f;
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime;
    u64 frameNumber = 0;

    Tilemap map = {};
    Cursor cursor;

    shared_ptr<Unit> lucina = make_shared<Unit>(0, true, 3, 2, 2, LoadTextureImage("../assets/sprites/lucina_final.png"));
    map.tiles[3][5].occupant = lucina;
    map.tiles[3][5].occupied = true;

    shared_ptr<Unit> donnel = make_shared<Unit>(1, true, 6, 1, 1, LoadTextureImage("../assets/sprites/donnel_final.png"));
    map.tiles[4][5].occupant = donnel;
    map.tiles[4][5].occupied = true;

    shared_ptr<Unit> flavia = make_shared<Unit>(2, false, 3, 1, 1, LoadTextureImage("../assets/sprites/flavia_final.png"));
    map.tiles[6][4].occupant = flavia;
    map.tiles[6][4].occupied = true;

    handler.BindUp(make_shared<MoveCommand>(&cursor, 0, -1, map));
    handler.BindDown(make_shared<MoveCommand>(&cursor, 0, 1, map));
    handler.BindLeft(make_shared<MoveCommand>(&cursor, -1, 0, map));
    handler.BindRight(make_shared<MoveCommand>(&cursor, 1, 0, map));
    handler.BindA(make_shared<OpenGameMenuCommand>());
    handler.BindB(make_shared<NullCommand>());

    unique_ptr<Texture> debugMessageOne = LoadTextureText("placeholder1", {250, 0, 0, 255});
    unique_ptr<Texture> debugMessageTwo = LoadTextureText("placeholder2", {0, 100, 0, 255});
    unique_ptr<Texture> debugMessageThree = LoadTextureText("placeholder3", {0, 0, 250, 255});

    GlobalRunning = true;
    while(GlobalRunning)
    {
        HandleEvents(&input);
        shared_ptr<Command> newCommand = handler.HandleInput(&input);

        if(newCommand)
        {
            commandQueue.push(newCommand);
            //printf("Pushed a new Command onto the Queue!\n");
            //printf("Queue Size: %lu\n", commandQueue.size());
        }

        // NOTE: This can be tied to anything. 
        //       Say, an animation finishing, or a thread returning.
        if(!commandQueue.empty())
        {
            commandQueue.front().get()->Execute();
            commandQueue.pop();
            handler.UpdateCommands(&cursor, &map);
        }

        Render(map, cursor, *debugMessageOne, *debugMessageTwo, *debugMessageThree);


// =========================== v debug messages v ============================================
        endTime = SDL_GetPerformanceCounter();
        real32 ElapsedMS = ((endTime - startTime) / (real32)SDL_GetPerformanceFrequency()) * 1000.0f;

        // Debug Messages
        char buffer[256];

        //cursor.PrintCursorState();
        //input.PrintInputState();
        //unit->sheet.PrintSpriteState();

        sprintf(buffer, "Tile <%d, %d> | Type: %d, Occupied: %d, Occupant: %p",
               cursor.col, cursor.row, map.tiles[cursor.col][cursor.row].type,
               map.tiles[cursor.col][cursor.row].occupied,
               (void *)map.tiles[cursor.col][cursor.row].occupant.get());
        debugMessageOne = LoadTextureText(string(buffer), {250, 0, 0, 255});

        sprintf(buffer, "Mode: %d", GlobalInterfaceState);
        debugMessageTwo = LoadTextureText(string(buffer), {0, 100, 0, 255});

        sprintf(buffer, "MS: %.02f, FPS: %d", ElapsedMS, (int)(1.0f / ElapsedMS * 1000.0f));
        debugMessageThree = LoadTextureText(string(buffer), {0, 0, 250, 255});


        if(ElapsedMS < TargetMillisecondsPerFrame)
        {
            SDL_Delay((int)(TargetMillisecondsPerFrame - ElapsedMS));
        }
        startTime = SDL_GetPerformanceCounter();
        frameNumber++;
// ============================ ^ debug messages ^ ================================================
    }
    Close();
    return 0;
}



// ================================ Helper Functions ======================================================

// ============================== loading data =================================
// Loads a Texture displaying the given text in the given color.
unique_ptr<Texture>
LoadTextureText(std::string text, SDL_Color color)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    assert(GlobalFont);
    surface = TTF_RenderText_Solid(GlobalFont, text.c_str(), color);
    assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    assert(texture);
    SDL_FreeSurface(surface);

    return make_unique<Texture>(texture, width, height);
}

// Loads a texture displaying an image, given a path to it.
shared_ptr<Texture>
LoadTextureImage(std::string path)
{
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;

    surface = IMG_Load(path.c_str());
    assert(surface);
    int width = surface->w;
    int height = surface->h;
    texture = SDL_CreateTextureFromSurface(GlobalRenderer, surface);
    assert(texture);
    SDL_FreeSurface(surface);

    return make_shared<Texture>(texture, width, height);
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
                            IMGUI_CHECKVERSION();
                            ImGui::CreateContext();
                            ImGuiIO& io = ImGui::GetIO(); (void)io;

     						// Enable Keyboard Control
                            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

						    // Enable Gamepad Controls
                            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

							printf("%d\n", io.WantCaptureKeyboard);
							io.Fonts->AddFontFromFileTTF("../assets/fonts/verdanab.ttf", 10.0f);

                            // Setup Dear ImGui style
                            ImGui::StyleColorsDark();

                            // Setup Platform/Renderer backends
                            ImGui_ImplSDL2_InitForSDLRenderer(GlobalWindow, GlobalRenderer);
                            ImGui_ImplSDLRenderer_Init(GlobalRenderer);

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

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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
			ImGui_ImplSDL2_ProcessEvent(&Event);
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
