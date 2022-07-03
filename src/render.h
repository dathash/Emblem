// Author: Alex Hartford
// Program: Emblem
// File: Render
// Date: July 2022

#ifndef RENDER_H
#define RENDER_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

// ================================ Rendering ==========================================
// Renders an individual tile to the screen, given its game coords and color.
void
RenderTile(int col, int row, const SDL_Color &color)
{
    SDL_Rect tileRect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    SDL_SetRenderDrawColor(GlobalRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(GlobalRenderer, &tileRect);
    SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(GlobalRenderer, &tileRect);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(int col, int row, 
             int colPixelOffset, int rowPixelOffset,
             const SpriteSheet &sheet)
{
    SDL_Rect destination = {col * TILE_SIZE + colPixelOffset, 
                            row * TILE_SIZE + rowPixelOffset, 
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.frame * sheet.size, sheet.track * sheet.size, sheet.size, sheet.size};

    SDL_RenderCopy(GlobalRenderer, sheet.texture->sdlTexture, &source, &destination);
}

// Renders a given texture at a pixel point.
// (Meant for debug text)
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

// ================================= render map tiles ============================================
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
        }
    }

// ================================= render selected or targeted =====================================
    if(GlobalInterfaceState == SELECTED_OVER_GROUND ||
       GlobalInterfaceState == SELECTED_OVER_INACCESSIBLE ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY)
    {
        SDL_Color accessibleColor = {0, 150, 0, 100};
        for(pair<int, int> cell : *map.accessible.get())
        {
            RenderTile(cell.first, cell.second, accessibleColor);
        }
    }

    if(GlobalInterfaceState == ATTACK_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == ATTACK_TARGETING_OVER_UNTARGETABLE)
    {
        SDL_Color attackColor = {250, 0, 0, 100};
        for(pair<int, int> cell : *map.interactible.get())
        {
            RenderTile(cell.first, cell.second, attackColor);
        }
    }

    if(GlobalInterfaceState == HEALING_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == HEALING_TARGETING_OVER_UNTARGETABLE)
    {
        SDL_Color healColor = {0, 100, 0, 100};
        for(pair<int, int> cell : *map.interactible.get())
        {
            RenderTile(cell.first, cell.second, healColor);
        }
    }


// ================================= render sprites ================================================
    for(int col = 0; col < MAP_SIZE; ++col)
    {
        for(int row = 0; row < MAP_SIZE; ++row)
        {
            const Tile *tileToRender = &map.tiles[col][row];

            if(tileToRender->occupied)
            {
                if(tileToRender->occupant->isExhausted)
                {
                    SDL_SetTextureColorMod(tileToRender->occupant->sheet->texture->sdlTexture, 50, 0, 0);
                }
                RenderSprite(col, row, 0, 0, *tileToRender->occupant->sheet);
            }
        }
    }


// ================================= render cursor ================================================
    RenderSprite(cursor.col, cursor.row, 
                 cursor.sheet->colPixelOffset, cursor.sheet->rowPixelOffset, 
                 *cursor.sheet);


// ================================= render ui elements ===========================================

    if(GlobalGuiMode)
    {
        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering ImGui
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    }


// ================================= render debug messages ========================================
    RenderText(debugMessageOne, 0, TILE_SIZE * MAP_SIZE);
    RenderText(debugMessageTwo, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height);
    RenderText(debugMessageThree, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height + debugMessageTwo.height);

    SDL_RenderPresent(GlobalRenderer);
}

#endif
