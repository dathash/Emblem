// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

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
                    GlobalFont = TTF_OpenFont("../assets/fonts/verdanab.ttf", 28);
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

                                io.Fonts->AddFontFromFileTTF("../assets/fonts/verdanab.ttf", 10.0f);

                                // Setup Dear ImGui style
                                ImGui::StyleColorsDark();

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

    //ImGui_ImplSDLRenderer_Shutdown();
    //ImGui_ImplSDL2_Shutdown();
    //ImGui::DestroyContext();

    ma_engine_uninit(&GlobalMusicEngine);

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
    if(input->joystickCooldown)
    {
        --input->joystickCooldown;
        printf("CD: %d\n", input->joystickCooldown);
    }
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

#endif
