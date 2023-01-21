// Author: Alex Hartford
// Program: Emblem
// File: Init

#ifndef INIT_H
#define INIT_H

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
                    GlobalFont = TTF_OpenFont("../assets/fonts/bjg.ttf", 20);
                    if(GlobalFont)
                    {
                        int imgFlags = IMG_INIT_PNG;
                        if(IMG_Init(imgFlags) & imgFlags)
                        {
                            if(ma_engine_init(NULL, &GlobalMusicEngine) == MA_SUCCESS)
                            {
                                // Setup Dear ImGui context
                                IMGUI_CHECKVERSION();
                                ImGui::CreateContext();
                                ImGuiIO& io = ImGui::GetIO(); (void)io;
                                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

                                io.Fonts->AddFontFromFileTTF("../assets/fonts/verdanab.ttf", 10.0f);
                                uiFontSmall = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 20.0f);
                                uiFontMedium = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 30.0f);
                                uiFontLarge = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 40.0f);

                                // Setup Dear ImGui style
                                ImGui::StyleColorsLight();

                                // Setup Platform/Renderer backends
                                ImGui_ImplSDL2_InitForSDLRenderer(GlobalWindow, GlobalRenderer);
                                ImGui_ImplSDLRenderer_Init(GlobalRenderer);

                                return true;
                            }
                            else
                            {
                            assert(!"ERROR: MINIAUDIO\n");
                            }
                        }
                        else
                        {
                        assert(!"ERROR: IMG\n");
                        }
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

    ma_engine_uninit(&GlobalMusicEngine);

    TTF_CloseFont(GlobalFont);
    SDL_DestroyRenderer(GlobalRenderer);
    SDL_DestroyWindow(GlobalWindow);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

#endif
