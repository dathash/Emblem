// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

#ifndef LOAD_H
#define LOAD_H

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

#endif
