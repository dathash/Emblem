// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

/*
    TODO
    Dijkstra's
    Commands Do Things

	NEXT
    Xbox Controller

	Unit Movement

	Enemy Turn
		Basic AI

	Level System
		Loading
		Win Condition

	Animation
		Sprite Animation
		Combat Scene
		Key Repeat

	Menu
	Music (MiniAudio)
	Undo
 */


// ========================= constants =====================================

// Rendering
#define TILE_SIZE 100
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 900

// Backend
#define MAP_SIZE 8



// ========================== includes =====================================
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <memory>
#include <string>
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
    int id;
	bool isAlly;
    shared_ptr<SpriteSheet> sheet;

    Unit(int id_in, bool isAlly_in, shared_ptr<Texture> texture_in)
    {
        //printf("Unit Constructed!\n");
        this->id = id_in;
        this->isAlly = isAlly_in;
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
    int type;
    bool occupied;
    std::shared_ptr<Unit> occupant;
};

struct Tilemap
{
    Tile tiles[MAP_SIZE][MAP_SIZE] = {};
};

struct Cursor
{
    int col = 1;
    int row = 1;
    std::shared_ptr<Unit> selected = nullptr;
    int selectedCol = -1;
    int selectedRow = -1;

    void PrintCursorState()
    {
        printf("====== Cursor ======\n");
        printf("Col : %d\n", col);
        printf("Row : %d\n", row);
        printf("Sel : %ld\n", (long int)selected.get());
    }
};

#include "command.h"

// ============================= globals ===================================
static SDL_Window *GlobalWindow = nullptr;
static SDL_Renderer *GlobalRenderer = nullptr;
static bool GlobalRunning = false;
static TTF_Font *GlobalFont = nullptr;



// ===================== function signatures ==============================
void Render(const Tilemap &map, const Cursor &cursor, const Texture &debugMessageOne, const Texture &debugMessageTwo, const Texture &debugMessageThree);
void RenderText(const Texture &texture, int x, int y);

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

    InputState input = {};
    InputHandler handler;
    queue<shared_ptr<Command>> commandQueue = {};

    real32 TargetMillisecondsPerFrame = 16.666f;
    u64 startTime = SDL_GetPerformanceCounter();
    u64 endTime;
    u64 frameNumber = 0;

    Tilemap map = {};
    Cursor cursor;

    shared_ptr<Unit> unit = make_shared<Unit>(0, true, LoadTextureImage("lucina_final.png"));
    map.tiles[3][5].occupant = unit;
    map.tiles[3][5].occupied = true;

    shared_ptr<Unit> enemy = make_shared<Unit>(1, false, LoadTextureImage("flavia_final.png"));
    map.tiles[6][4].occupant = enemy;
    map.tiles[6][4].occupied = true;

    handler.BindUp(make_shared<MoveCursorCommand>(&cursor, 0, -1, map));
    handler.BindDown(make_shared<MoveCursorCommand>(&cursor, 0, 1, map));
    handler.BindLeft(make_shared<MoveCursorCommand>(&cursor, -1, 0, map));
    handler.BindRight(make_shared<MoveCursorCommand>(&cursor, 1, 0, map));
    handler.BindA(make_shared<OpenMenuCommand>());
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


// ============================ debug messages ===============================================
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
// ============================ ^ DEBUG ^ ================================================
    }
    Close();
    return 0;
}



// ================================ Helper Functions ======================================================


// ================================ Rendering ==========================================
// Renders an individual tile to the screen, given its game coords and color.
void
RenderTile(int col, int row, const SDL_Color &color)
{
    SDL_Rect Rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    SDL_SetRenderDrawColor(GlobalRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(GlobalRenderer, &Rect);
    SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(GlobalRenderer, &Rect);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(int col, int row, const SpriteSheet &sheet)
{
    SDL_Rect destination = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.col * sheet.size, sheet.row * sheet.size, sheet.size, sheet.size};

    SDL_RenderCopy(GlobalRenderer, sheet.texture->sdlTexture, &source, &destination);
}

void
RenderText(const Texture &texture, int x, int y)
{
    SDL_Rect destination = {x, y, texture.width, texture.height};
    SDL_RenderCopy(GlobalRenderer, texture.sdlTexture, NULL, &destination);
}

// Renders the scene from the given game state.
void
Render(const Tilemap &map, const Cursor &cursor, 
       const Texture &debugMessageOne, 
       const Texture &debugMessageTwo, 
       const Texture &debugMessageThree)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, 200, 200, 0, 255);
    SDL_RenderClear(GlobalRenderer);

    // Render Map
    for(int col = 0; col < MAP_SIZE; ++col)
    {
        for(int row = 0; row < MAP_SIZE; ++row)
        {
            SDL_Color tileColor = {};
            const Tile *tileToRender = &map.tiles[col][row];
            switch(tileToRender->type)
            {
                case(FLOOR):
                {
                    tileColor = {255, 255, 255, 255};
                } break;
                case(WALL):
                {
                    tileColor = {50, 50, 50, 255};
                } break;

                default:
                {} break;
            }

            RenderTile(col, row, tileColor);

            if(tileToRender->occupant)
            {
                RenderSprite(col, row, *tileToRender->occupant->sheet);
            }
        }
    }

    // Render Cursor
    SDL_Color cursorColor = {0, 150, 0, 100};
    RenderTile(cursor.col, cursor.row, cursorColor);

    RenderText(debugMessageOne, 0, TILE_SIZE * MAP_SIZE);
    RenderText(debugMessageTwo, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height);
    RenderText(debugMessageThree, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height + debugMessageTwo.height);

    SDL_RenderPresent(GlobalRenderer);
}



// =================================== Game Logic =================================================


// Mutates the given Cursor to move it.
void
MoveCursor(Cursor *cursor, int colDelta, int rowDelta)
{
	int newCol = cursor->col + colDelta;
	int newRow = cursor->row + rowDelta;
	if(IsValidBoundsPosition(newCol, newRow))
	{
		cursor->col = newCol;
		cursor->row = newRow;
	}
}

// Mutates a tile's type at the cursor's position.
void
ChangeTileType(const Cursor &cursor, Tilemap *map, int type)
{
	map->tiles[cursor.col][cursor.row].type = type;
}


// Selects a given tile.
void
Select(const Tilemap &map, Cursor *cursor)
{
	cursor->selected = map.tiles[cursor->col][cursor->row].occupant;
}


// Deselects the currently selected tile.
void
Deselect(Cursor *cursor)
{
	cursor->selected = nullptr;
}


// ============================== Loading Data =================================
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

    return make_shared<Texture>(texture, width, height);;
}


// ================================== SDL Functions ================================================

// Initializes SDL Objects.
bool
Initialize()
{
    if(SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        GlobalWindow = SDL_CreateWindow("Emblem",
                                        700,
                                        200,
                                        SCREEN_WIDTH,
                                        SCREEN_HEIGHT,
                                        SDL_WINDOW_SHOWN);
        if(GlobalWindow)
        {
            GlobalRenderer = SDL_CreateRenderer(GlobalWindow, -1, SDL_RENDERER_ACCELERATED);
            if(GlobalRenderer)
            {
                if(TTF_Init() != -1)
                {
                    GlobalFont = TTF_OpenFont("verdanab.ttf", 28);
                    if(GlobalFont)
                    {
                        int imgFlags = IMG_INIT_PNG;
                        if(IMG_Init(imgFlags) & imgFlags)
                        {
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
        if(Event.type == SDL_QUIT)
        {
            GlobalRunning = false;
        }
        else if(Event.type == SDL_KEYDOWN && Event.key.repeat == 0)
        {
            switch(Event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                {
                    GlobalRunning = false;
                } break;

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
    }
}
