// Author: Alex Hartford
// Program: Emblem
// File: Render

#ifndef RENDER_H
#define RENDER_H

// ================================ Rendering ==========================================
// Renders an individual tile to the screen, given its game coords and color.
void
RenderTileColor(const position &pos, const SDL_Color &color)
{
    //assert(col >= 0 && row >= 0);
    SDL_Rect tileRect = {TILE_SIZE / 16 + pos.col * TILE_SIZE, TILE_SIZE / 16 + pos.row * TILE_SIZE, 
                         TILE_SIZE - (TILE_SIZE / 16) * 2, 
                         TILE_SIZE - (TILE_SIZE / 16) * 2};

    SDL_SetRenderDrawColor(GlobalRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(GlobalRenderer, &tileRect);
    SDL_SetRenderDrawColor(GlobalRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(GlobalRenderer, &tileRect);
}

// Renders an individual tile to the screen, given its game coords and tile (for texture).
void
RenderTileTexture(const Tilemap &map, const Tile &tile,
                  const position &pos)
{
    SDL_Rect destination = {pos.col * TILE_SIZE, pos.row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {tile.atlas_index.col * map.atlas_tile_size, 
                       tile.atlas_index.row * map.atlas_tile_size, 
                       map.atlas_tile_size, map.atlas_tile_size};
    SDL_RenderCopy(GlobalRenderer, map.atlas.sdl_texture, &source, &destination);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(const position &pos, const Spritesheet &sheet, bool flipped = false,
             const position &animation_offset = position(0, 0))
{
    SDL_Rect destination = {pos.col * TILE_SIZE + animation_offset.col, 
                            pos.row * TILE_SIZE + animation_offset.row, 
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.frame * sheet.size, sheet.track * sheet.size, sheet.size, sheet.size};

    SDL_RenderCopyEx(GlobalRenderer, sheet.texture.sdl_texture, &source, &destination,
                     0, NULL, (const SDL_RendererFlip)flipped);
}

// Renders a unit's portrait.
void
RenderPortrait(int x, int y, const Texture &portrait, bool flipped)
{
    SDL_Rect destination = {x, y, 
                            PORTRAIT_SIZE,
                            PORTRAIT_SIZE};
    SDL_Rect source = {0, 0, portrait.width, portrait.height};

    SDL_RenderCopyEx(GlobalRenderer, portrait.sdl_texture, &source, &destination, 
                     0, NULL, (const SDL_RendererFlip)flipped);
}

// Renders a given texture at a pixel point.
// (Meant for debug text)
void
RenderText(const Texture &texture, int x, int y)
{
    SDL_Rect destination = {x, y, texture.width, texture.height};
    SDL_RenderCopy(GlobalRenderer, texture.sdl_texture, NULL, &destination);
}

// Renders a Health Bar (For use underneath units)
void
RenderHealthBarSmall(const position &p, int hp, int maxHp)
{
    float ratio = (float)hp / maxHp;
    SDL_Color healthColor = PiecewiseColors(red, yellow, green, ratio);

    SDL_Rect bar_rect = {p.col * TILE_SIZE + 7, 
                            p.row * TILE_SIZE + 50,
                            50, 8};
    SDL_Rect health_rect = {p.col * TILE_SIZE + 7,
                            p.row * TILE_SIZE + 50,
                            (int)(50 * ratio), 8};

    SDL_SetRenderDrawColor(GlobalRenderer, darkGray.r, darkGray.g, darkGray.b, darkGray.a);
    SDL_RenderFillRect(GlobalRenderer, &bar_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, healthColor.r, healthColor.g, healthColor.b, healthColor.a);
    SDL_RenderFillRect(GlobalRenderer, &health_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
    SDL_RenderDrawRect(GlobalRenderer, &bar_rect);
}

// Renders a Health Bar (For Combat Animations)
void
RenderHealthBarCombat(const position &p, int hp, int maxHp)
{
    float ratio = (float)hp / maxHp;
    SDL_Color healthColor = PiecewiseColors(red, yellow, green, ratio);

    SDL_Rect bar_rect = {p.col, p.row,
                         200, 24};
    SDL_Rect health_rect = {p.col, p.row,
                            (int)(200 * ratio), 24};

    SDL_SetRenderDrawColor(GlobalRenderer, darkGray.r, darkGray.g, darkGray.b, darkGray.a);
    SDL_RenderFillRect(GlobalRenderer, &bar_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, healthColor.r, healthColor.g, healthColor.b, healthColor.a);
    SDL_RenderFillRect(GlobalRenderer, &health_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
    SDL_RenderDrawRect(GlobalRenderer, &bar_rect);
}

// Renders the scene from the given game state.
void
Render(const Tilemap &map, const Cursor &cursor, 
       const Menu &gameMenu, const Menu &unitMenu, const Menu &levelMenu,
       const Conversation &conversation, const Fight &fight)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(GlobalRenderer);

// ================================= render map tiles ============================================
    for(int col = viewportCol; col < VIEWPORT_WIDTH + viewportCol; ++col)
    {
        for(int row = viewportRow; row < VIEWPORT_HEIGHT + viewportRow; ++row)
        {
            position screen_pos = {col - viewportCol, row - viewportRow};
            const Tile &tile = map.tiles[col][row];
            RenderTileTexture(map, tile, screen_pos);
            if(tile.type == SPAWN && GlobalEditorMode)
                RenderTileColor(screen_pos, yellow);
        }
    }

// ================================= render selected or targeted =====================================
    if(GlobalInterfaceState == SELECTED_OVER_GROUND ||
       GlobalInterfaceState == SELECTED_OVER_INACCESSIBLE ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY)
    {
        for(const position &cell : map.accessible)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor(
                        {cell.col - viewportCol, cell.row - viewportRow},
                        moveColor
                        );
            }
        }

        for(const position &p : cursor.path_draw)
        {
            RenderTileColor({p.col - viewportCol, 
                       p.row - viewportRow}, 
                       pathColor);
        }
    }

    if(GlobalInterfaceState == ENEMY_RANGE)
    {
        for(const position &cell : map.accessible)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           aiMoveColor);
            }
        }
    }

    if(GlobalInterfaceState == ATTACK_TARGETING)
    {
        for(const position &cell : map.range)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           attackColor);
            }
        }

        for(const position &cell : map.attackable)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           yellow);
            }
        }
    }

    if(GlobalInterfaceState == HEAL_TARGETING)
    {
        for(const position &cell : map.range)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           healColor);
            }
        }

        for(const position &cell : map.healable)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           yellow);
            }
        }
    }

// ================================ ai visualization  =============================
    if(GlobalAIState == SELECTED)
    {
        for(const position &cell : map.accessible)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           aiMoveColor);
            }
        }
    }

// ================================= render sprites ================================================
    for(int col = viewportCol; col < VIEWPORT_WIDTH + viewportCol; ++col)
    {
        for(int row = viewportRow; row < VIEWPORT_HEIGHT + viewportRow; ++row)
        {
            const Tile &tileToRender = map.tiles[col][row];
            if(tileToRender.occupant)
            {
                if(tileToRender.occupant->is_exhausted)
                {
                    SDL_SetTextureColorMod(tileToRender.occupant->sheet.texture.sdl_texture, exhaustedMod.r, exhaustedMod.g, exhaustedMod.b);
                }
                else
                {
                    SDL_SetTextureColorMod(tileToRender.occupant->sheet.texture.sdl_texture, readyMod.r, readyMod.g, readyMod.b);
                }
                position screen_pos = {col - viewportCol, row - viewportRow};
                RenderSprite(screen_pos, tileToRender.occupant->sheet, tileToRender.occupant->is_ally, tileToRender.occupant->animation_offset);
                RenderHealthBarSmall(screen_pos, tileToRender.occupant->health, tileToRender.occupant->max_health);
            }
        }
    }


// ================================= render cursor ================================================
    if(GlobalInterfaceState != PLAYER_FIGHT &&
       GlobalAIState != AI_FIGHT)
    {
        RenderSprite(cursor.pos - position(viewportCol, viewportRow), cursor.sheet);
    }


// ==================================== menus =====================================================
    // Game Menu
    if(GlobalInterfaceState == GAME_MENU)
    {
        for(int i = 0; i < gameMenu.rows; ++i)
        {
            SDL_Rect menuRect = {650, 140 + i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(gameMenu.optionTextTextures[i], menuRect.x + 10, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {650, 140 + MENU_ROW_HEIGHT * gameMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    // Unit Menu
    if(GlobalInterfaceState == UNIT_MENU_ROOT)
    {
        for(int i = 0; i < unitMenu.rows; ++i)
        {
            SDL_Rect menuRect = {650, 140 + i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(unitMenu.optionTextTextures[i], menuRect.x + 10, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {650, 140 + MENU_ROW_HEIGHT * unitMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    if(GlobalInterfaceState == LEVEL_MENU)
    {
        for(int i = 0; i < levelMenu.rows; ++i)
        {
            SDL_Rect menuRect = {650, 140 + i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(levelMenu.optionTextTextures[i], menuRect.x + 10, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {650, 140 + MENU_ROW_HEIGHT * levelMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    /*
    // Portraits
    if((GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
       GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
       GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY ||
       GlobalInterfaceState == UNIT_MENU_ROOT ||
	   GlobalInterfaceState == UNIT_INFO ||
	   GlobalInterfaceState == ENEMY_INFO)
       && GlobalPlayerTurn && !GlobalEditorMode)
    {
        assert(map.tiles[cursor.pos.col][cursor.pos.row].occupant);
        int x_pos = 560;
        if(map.tiles[cursor.pos.col][cursor.pos.row].occupant->is_ally)
            x_pos = 180;

        RenderPortrait(x_pos, 300, map.tiles[cursor.pos.col][cursor.pos.row].occupant->portrait,
                       map.tiles[cursor.pos.col][cursor.pos.row].occupant->is_ally);
    }
    */

    if(GlobalInterfaceState == PREVIEW_ATTACK ||
	   GlobalInterfaceState == PREVIEW_HEALING)
    {
        assert(cursor.selected);

        RenderPortrait(30, 0, cursor.selected->portrait, true);
        RenderPortrait(430, 0, cursor.targeted->portrait, false);
}

    if(GlobalInterfaceState == CONVERSATION)
    {
        RenderPortrait(50, 0, conversation.one.portrait,
                       true);
        RenderPortrait(500, 0, conversation.two.portrait,
                       false);

        SDL_Rect conv_rect = {20, 400, 860, 180};
        SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
        SDL_RenderFillRect(GlobalRenderer, &conv_rect);
        SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &conv_rect);

        SDL_Rect name_rect;
        if(conversation.Speaker() == SPEAKER_ONE)
        {
            name_rect = {20, 400, 300, 40};
        }
        else
        {
            name_rect = {580, 400, 300, 40};
        }
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, 255);
        SDL_RenderFillRect(GlobalRenderer, &name_rect);
        SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &name_rect);

        RenderText(conversation.speaker_texture, name_rect.x + 10, 400);
        RenderText(conversation.words_texture, 30, 440);
    }
}

#endif
