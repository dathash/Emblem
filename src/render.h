// Author: Alex Hartford
// Program: Emblem
// File: Render
// Date: July 2022

#ifndef RENDER_H
#define RENDER_H

//#include "imgui.h"
//#include "imgui_impl_sdl.h"
//#include "imgui_impl_sdlrenderer.h"

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
       const Menu &gameMenu, const Menu &unitMenu,
       const UnitInfo &unitInfo,
       const CombatInfo &combatInfo,
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
        SDL_Color healColor = {0, 255, 0, 100};
        for(pair<int, int> cell : *map.interactible.get())
        {
            RenderTile(cell.first, cell.second, healColor);
        }
    }

    if(GlobalInterfaceState == TRADING_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == TRADING_TARGETING_OVER_UNTARGETABLE)
    {
        SDL_Color healColor = {0, 0, 250, 100};
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
    const SDL_Color uiColor = {60, 60, 150, 250};
    const SDL_Color uiTextColor = {255, 255, 255, 255};
    const SDL_Color uiSelectorColor = {50, 50, 50, 100};

// ==================================== menus =====================================================
    if(GlobalInterfaceState == GAME_MENU_ROOT)
    {
        for(int i = 0; i < gameMenu.rows; ++i)
        {
            SDL_Rect menuRect = {TILE_SIZE * MAP_SIZE, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(*gameMenu.optionTextTextures[i], menuRect.x, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {TILE_SIZE * MAP_SIZE, MENU_ROW_HEIGHT * gameMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, uiTextColor.r, uiTextColor.g, uiTextColor.b, uiTextColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    else if(GlobalInterfaceState == UNIT_MENU_ROOT)
    {
        for(int i = 0; i < unitMenu.rows; ++i)
        {
            SDL_Rect menuRect = {TILE_SIZE * MAP_SIZE, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(*unitMenu.optionTextTextures[i], menuRect.x, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {TILE_SIZE * MAP_SIZE, MENU_ROW_HEIGHT * unitMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, uiTextColor.r, uiTextColor.g, uiTextColor.b, uiTextColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

// ================================== info pages ===============================================
    else if(GlobalInterfaceState == UNIT_INFO ||
            GlobalInterfaceState == ENEMY_INFO)
    {
        for(int i = 0; i < unitInfo.rows; ++i)
        {
            SDL_Rect infoRect = {TILE_SIZE * MAP_SIZE, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &infoRect);
            SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(GlobalRenderer, &infoRect);

            RenderText(*unitInfo.infoTextTextures[i], infoRect.x, infoRect.y);
        }
    }
    else if(GlobalInterfaceState == PREVIEW_ATTACK ||
            GlobalInterfaceState == PREVIEW_HEALING)
    {
        for(int i = 0; i < combatInfo.rows; ++i)
        {
            SDL_Rect sourceRect = {TILE_SIZE * MAP_SIZE, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_Rect targetRect = {TILE_SIZE * MAP_SIZE + MENU_WIDTH, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &sourceRect);
            SDL_RenderFillRect(GlobalRenderer, &targetRect);
            SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(GlobalRenderer, &sourceRect);
            SDL_RenderDrawRect(GlobalRenderer, &targetRect);

            RenderText(*combatInfo.sourceTextTextures[i], sourceRect.x, sourceRect.y);
            RenderText(*combatInfo.targetTextTextures[i], targetRect.x, targetRect.y);
        }
    }


    if(GlobalGuiMode)
    {
        //ImGui_ImplSDLRenderer_NewFrame();
        //ImGui_ImplSDL2_NewFrame();
        //ImGui::NewFrame();
        //{
            //static float f = 0.0f;
            //static int counter = 0;

            //ImGui::Begin("Hello, world!");

            //ImGui::Text("This is some useful text.");

            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            //if (ImGui::Button("Button"))
                //counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::End();
        //}
        //ImGui::Render();
        //ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    }


// ================================= render debug messages ========================================
    RenderText(debugMessageOne, 0, TILE_SIZE * MAP_SIZE);
    RenderText(debugMessageTwo, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height);
    RenderText(debugMessageThree, 0, TILE_SIZE * MAP_SIZE + debugMessageOne.height + debugMessageTwo.height);

    SDL_RenderPresent(GlobalRenderer);
}

#endif
