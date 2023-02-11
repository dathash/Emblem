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
    // SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
        return false;

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
    if(!GlobalWindow)
        return false;

    GlobalRenderer = SDL_CreateRenderer(GlobalWindow, -1, 
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if(!GlobalRenderer)
        return false;

    // TTF
    if(TTF_Init() == -1)
        return false;
    GlobalFont = TTF_OpenFont("../assets/fonts/bjg.ttf", 20);
    if(!GlobalFont)
        return false;

    // IMG
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags))
        return false;

    // MINIAUDIO
    if(ma_engine_init(NULL, &GlobalAudioEngine) != MA_SUCCESS)
        return false;

    ma_sound_group_init(&GlobalAudioEngine, 0, nullptr, &(GlobalMusicGroup));
    ma_sound_group_init(&GlobalAudioEngine, 0, nullptr, &(GlobalSfxGroup));
    SetMusicVolume(DEFAULT_MUSIC_VOLUME);
    SetSfxVolume(DEFAULT_SFX_VOLUME);

    // DEAR IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    io.Fonts->AddFontFromFileTTF("../assets/fonts/verdanab.ttf", 10.0f);
    uiFontSmall = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 20.0f);
    uiFontMedium = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 30.0f);
    uiFontLarge = io.Fonts->AddFontFromFileTTF("../assets/fonts/bjg.ttf", 40.0f);

    ImGui::StyleColorsLight();

    ImGui_ImplSDL2_InitForSDLRenderer(GlobalWindow, GlobalRenderer);
    ImGui_ImplSDLRenderer_Init(GlobalRenderer);

    return true;
}

// Frees up allocated memory at the end of the program.
// Note I don't call destroy on all the textures and stuff
// because the OS will do this for me, and it will needlessly
// slow down shutdown.
//
// NOTE: I don't know this for sure, I've just heard about it
// before.
void Close()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    ma_sound_group_uninit(&GlobalMusicGroup);
    ma_sound_group_uninit(&GlobalSfxGroup);
    ma_engine_uninit(&GlobalAudioEngine);

    TTF_CloseFont(GlobalFont);
    SDL_DestroyRenderer(GlobalRenderer);
    SDL_DestroyWindow(GlobalWindow);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

#endif
