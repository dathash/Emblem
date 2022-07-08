// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

/*
    TODO
	MVP
		Turns
			Enemy Turn
			Basic AI
        Level Transitions
        Death
        New Textures
		Three main Units
		Create some cool levels


    NICE
		User Interface
			Show Tile details
			Items and Trading Menus
		Animation
			Combat Scene
			Placing Unit moves them to that square (A-star)
			Generate Arrow while selecting?
		Items
			Equipping
			Trading
			Different Ranges / Action types

    BACKLOG
        Better Level Editor
		Conversations
		Music (MiniAudio)
		Items
		Replay!
		Smooth Interaction Syntax (Just click on enemy to attack them)
		Switch to GameController API
		Key Repeat
		Tiles have properties 
			(That remain together and don't rely on eachother)
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
#define JOYSTICK_DEAD_ZONE 8000

#define TIME_STEP 33.33333

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



// TODO MOVE THIS
static bool GlobalTweening = false;
static bool ReadyForCommand = false;



// ============================= globals ===================================
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static bool GlobalRunning = false;
static bool GlobalGamepadMode = false;
static bool GlobalGuiMode = false;

static TTF_Font *GlobalFont = nullptr;



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
            assert(!"SpriteSheet | ChooseTrack | Invalid Animation Track!\n");
        }
        this->track = track_in;
        this->frame = 0;
    }

    SpriteSheet()
    {
        //printf("SpriteSheet Constructed! (Default)\n");
    }

    SpriteSheet(shared_ptr<Texture> texture_in, int size_in, int speed_in)
    {
        this->texture = texture_in;
        this->size = size_in;
        this->speed = speed_in;

        this->tracks = texture_in->height / size_in;
        this->frames = texture_in->width / size_in;
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
        printf("t's : %d\n", tracks);
        printf("f's : %d\n", frames);
        printf("trak: %d\n", track);
        printf("fram: %d\n", frame);
        printf("spd : %d\n", speed);
        printf("cnt : %d\n", counter);
    }
};

struct Tween
{
    int start;
    int end;
    int duration;
    int currentTime = 0;
    bool active = false;
    bool resetting = false;

    int *value;

    Tween(int start_in, int end_in, int duration_in, int *value_in, bool resetting_in)
    : start(start_in),
      end(end_in),
      duration(duration_in),
      value(value_in),
      resetting(resetting_in)
    {}

    void Activate()
    {
        active = true;
        GlobalTweening = true;
    }

    void Deactivate()
    {
        active = false;
        GlobalTweening = false;
        ReadyForCommand = true;
        if(resetting)
        {
            *value = start;
        }
    }

    void Reset()
    {
        currentTime = 0;
        active = true;
    }

    void Update()
    {
        ++this->currentTime;
        if(currentTime < duration)
        {
            *value = Position();
        }
        else
        {
            Deactivate();
        }
    }

    int Lerp(int a, int b, float amount)
    {
        return (int)((1.0f - amount) * a + amount * b);
    }

    int Position()
    {
        float amount = (float)currentTime / (float)duration;
        return Lerp(start, end, amount);
    }
};


struct Unit
{
    string name;
    int id;
    bool shouldDie = false;
    bool isAlly = false;
    bool isExhausted = false;
    int mov = 0;
    int hp = 10;
    int maxHp = 10;
    int attack = 1;
    int defense = 1;
    int healing = 1;
    int minRange = 1;
    int maxRange = 1;
    int accuracy = 50;
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
        //printf("Unit Constructed!\n");
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

    void PrintCursorState()
    {
        printf("====== Cursor ======\n");
        printf("Col : %d\n", col);
        printf("Row : %d\n", row);
        printf("Sel : %ld\n", (long int)selected.get());
    }
};

// TODO: Move!!!
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

int d100()
{
    return rand() % 100;
}

void SimulateCombat(Unit *one, Unit *two)
{
    // one -> two
    int damage = one->attack;
    if(d100() < one->accuracy)
    {
        printf("SIMULATION | %d Damage!\n", damage);
        if(two->hp - damage <= 0)
        {
            two->hp = 0;
            two->shouldDie = true;
        }
        else
        {
            two->hp -= damage;
        }
    }
    else
    {
        printf("SIMULATION | Missed!\n");
    }

    // two -> one
    damage = two->attack;
    if(d100() > two->accuracy)
    {
        printf("SIMULATION | %d Damage!\n", damage);
        if(one->hp - damage <= 0)
        {
            one->hp = 0;
            one->shouldDie = true;
        }
        else
        {
            one->hp -= damage;
        }
    }
    else
    {
        printf("SIMULATION | Missed!\n");
    }
}

void SimulateHealing(Unit *one, Unit *two)
{
    // one -> two
    int healing = one->healing;
    printf("SIMULATION | %d Healing!\n", healing);
    two->hp = min(healing + two->hp, two->maxHp);
}


#include "grid.h"
#include "command.h"
#include "render.h"

// ===================== function signatures ==============================
bool Initialize();
void Close();
void HandleEvents(InputState *input);



// ================================== main =================================
int main(int argc, char *argv[])
{
    srand (time(NULL));

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
    shared_ptr<Tween> currentTween = nullptr;

    real32 TargetMillisecondsPerFrame = 16.666f;
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime = 0;
    u64 frameNumber = 0;
    real32 ElapsedMS = 0.0f;

    // load data
    vector<shared_ptr<Unit>> units = LoadCharacters("../data/units.txt");
    shared_ptr<Tilemap> map = LoadLevel("../data/l1.txt", units);

    Cursor cursor(LoadTextureImage("../assets/sprites/cursor.png"));;

    // Initial InputHandler commands
    handler.BindUp(make_shared<MoveCommand>(&cursor, 0, -1, *map));
    handler.BindDown(make_shared<MoveCommand>(&cursor, 0, 1, *map));
    handler.BindLeft(make_shared<MoveCommand>(&cursor, -1, 0, *map));
    handler.BindRight(make_shared<MoveCommand>(&cursor, 1, 0, *map));
    handler.BindA(make_shared<OpenGameMenuCommand>());
    handler.BindB(make_shared<NullCommand>());

    // Initialize Menus
    Menu gameMenu(3, 0, {"Outlook", "Options", "End Turn"});
    Menu unitMenu(6, 0, {"Info", "Items", "Attack", "Heal", "Trade", "Wait"});

    UnitInfo unitInfo(1, {"Placeholder"});
    CombatInfo combatInfo(1, {"Placeholder"}, {"Placeholder"});

    unique_ptr<Texture> debugMessageOne = LoadTextureText("placeholder1", {250, 0, 0, 255});
    unique_ptr<Texture> debugMessageTwo = LoadTextureText("placeholder2", {0, 100, 0, 255});
    unique_ptr<Texture> debugMessageThree = LoadTextureText("placeholder3", {0, 0, 250, 255});

    GlobalRunning = true;
    while(GlobalRunning)
    {
        HandleEvents(&input);

// ====================== Where the action Happens ===============
        if(GlobalTweening)
        {
            currentTween->Update();
        }
        else
        {
            shared_ptr<Command> newCommand = handler.HandleInput(&input);
            if(newCommand)
            {
                commandQueue.push(newCommand);
            }

            if(!commandQueue.empty())
            {
                currentTween = commandQueue.front()->GetTween();
                if(currentTween)
                {
                    currentTween->Activate();
                    GlobalTweening = true;
                }
                else
                {
                    ReadyForCommand = true;
                }
            }
        }

        if(ReadyForCommand)
        {
            commandQueue.front()->Execute();
            commandQueue.pop();
            handler.UpdateCommands(&cursor, map.get(),
                                   &gameMenu, &unitMenu, 
                                   &unitInfo, &combatInfo);
            if(currentTween)
            {
                currentTween->Reset();
            }
            ReadyForCommand = false;
        }
// ================================================================

        for(shared_ptr<Unit> unit : units)
        {
            unit->Update();
        }
        cursor.Update();


// ============================= render =========================================
        Render(*map, cursor, gameMenu, unitMenu, unitInfo, combatInfo,
               *debugMessageOne, *debugMessageTwo, *debugMessageThree);


// =========================== v debug messages v ============================================
        endTime = SDL_GetPerformanceCounter();
        ElapsedMS = ((endTime - startTime) / (real32)SDL_GetPerformanceFrequency()) * 1000.0f;

        // Debug Messages
        char buffer[256];

        //cursor.PrintCursorState();
        //input.PrintInputState();
        //unit->sheet.PrintSpriteState();

        //sprintf(buffer, "Tile <%d, %d> | Type: %d, Occupied: %d, Occupant: %p",
               //cursor.col, cursor.row, map.tiles[cursor.col][cursor.row].type,
               //map.tiles[cursor.col][cursor.row].occupied,
               //(void *)map.tiles[cursor.col][cursor.row].occupant.get());
        sprintf(buffer, "Mode: %d", GlobalInterfaceState);

        debugMessageOne = LoadTextureText(string(buffer), {250, 0, 0, 255});

        if(currentTween)
        {
            sprintf(buffer, "GlobalTweening: %d | Start: %d, End: %d, Current: %d, Value: %d",
                    GlobalTweening, currentTween->start, currentTween->end,
                    currentTween->currentTime, *currentTween->value);
        }
        else
        {
            sprintf(buffer, "GlobalTweening: %d | No Tween ATM.", GlobalTweening);
        }
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