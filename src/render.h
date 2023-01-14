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
RenderSprite(const position &pos, const Spritesheet &sheet, bool flipped = false)
{
    SDL_Rect destination = {pos.col * TILE_SIZE, 
                            pos.row * TILE_SIZE, 
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

// Renders a Health Bar
void
RenderHealthBar(int x, int y, int hp, int maxHp, bool allyColor)
{
    SDL_Rect healthRect;
    for(int i = 0; i < maxHp; ++i)
    {
        SDL_Rect healthRect = {x + HEALTH_TICK_WIDTH * i, y, HEALTH_TICK_WIDTH, HEALTH_TICK_HEIGHT};
        if(i < hp)
        {
            if(allyColor)
            {
                SDL_SetRenderDrawColor(GlobalRenderer, allyHealthBarColor.r, allyHealthBarColor.g, allyHealthBarColor.b, allyHealthBarColor.a);
            }
            else
            {
                SDL_SetRenderDrawColor(GlobalRenderer, enemyHealthBarColor.r, enemyHealthBarColor.g, enemyHealthBarColor.b, enemyHealthBarColor.a);
            }
        }
        else
        {
            SDL_SetRenderDrawColor(GlobalRenderer, healthBarLostColor.r, healthBarLostColor.g, healthBarLostColor.b, healthBarLostColor.a);
        }
        SDL_RenderFillRect(GlobalRenderer, &healthRect);
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &healthRect);
    }
}

// Renders a Health Bar, with a damage amount taken out of it.
void
RenderHealthBarDamage(int x, int y, int hp, int maxHp, int Dmg, bool allyColor)
{
    SDL_Rect healthRect;
    for(int i = 0; i < maxHp; ++i)
    {
        SDL_Rect healthRect = {x + HEALTH_TICK_WIDTH * i, y, HEALTH_TICK_WIDTH, HEALTH_TICK_HEIGHT};
        if(i < hp - Dmg)
        {
            if(allyColor)
            {
                SDL_SetRenderDrawColor(GlobalRenderer, allyHealthBarColor.r, allyHealthBarColor.g, allyHealthBarColor.b, allyHealthBarColor.a);
            }
            else
            {
                SDL_SetRenderDrawColor(GlobalRenderer, enemyHealthBarColor.r, enemyHealthBarColor.g, enemyHealthBarColor.b, enemyHealthBarColor.a);
            }
        }
        else if(i < hp)
        {
            if(allyColor)
            {
                SDL_SetRenderDrawColor(GlobalRenderer, allyHealthBarLosingColor.r, allyHealthBarLosingColor.g, allyHealthBarLosingColor.b, allyHealthBarLosingColor.a);
            }
            else
            {
                SDL_SetRenderDrawColor(GlobalRenderer, enemyHealthBarLosingColor.r, enemyHealthBarColor.g, enemyHealthBarLosingColor.b, enemyHealthBarLosingColor.a);
            }
        }
        else
        {
            SDL_SetRenderDrawColor(GlobalRenderer, healthBarLostColor.r, healthBarLostColor.g, healthBarLostColor.b, healthBarLostColor.a);
        }
        SDL_RenderFillRect(GlobalRenderer, &healthRect);
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &healthRect);
    }
}


// Renders the scene from the given game state.
void
Render(const Tilemap &map, const Cursor &cursor, 
       const Menu &gameMenu, const Menu &unitMenu, const Menu &levelMenu,
       const Conversation &conversation)
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
        for(const position &cell : map.attackable)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           attackColor);
            }
        }
    }

    if(GlobalInterfaceState == HEAL_TARGETING)
    {
        for(const position &cell : map.healable)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           healColor);
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
                RenderSprite(screen_pos, tileToRender.occupant->sheet, tileToRender.occupant->is_ally);
            }
        }
    }


// ================================= render cursor ================================================
    RenderSprite(cursor.pos - position(viewportCol, viewportRow), cursor.sheet, false);


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

    // Portraits
    if((GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
       GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
       GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY ||
       GlobalInterfaceState == ATTACK_TARGETING ||
       GlobalInterfaceState == HEAL_TARGETING ||
       GlobalInterfaceState == UNIT_MENU_ROOT ||
	   GlobalInterfaceState == UNIT_INFO ||
	   GlobalInterfaceState == ENEMY_INFO)
       && GlobalPlayerTurn && !GlobalEditorMode)
    {
        //cout << cursor.pos << "\n"; // TODO: There's an unlucky seg fault here. Investigate when exiting editor mode.
        assert(map.tiles[cursor.pos.col][cursor.pos.row].occupant);
        int x_pos = 560;
        if(map.tiles[cursor.pos.col][cursor.pos.row].occupant->is_ally)
            x_pos = 180;

        RenderPortrait(x_pos, 300, map.tiles[cursor.pos.col][cursor.pos.row].occupant->portrait,
                       map.tiles[cursor.pos.col][cursor.pos.row].occupant->is_ally);
    }

    // Portraits
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

    /*
	// Combat Screen
    // TODO: Add in barebones of the actual combat screen
    if(combatResolver.inc < (float)combatResolver.framesActive * 0.6666)
    {
        RenderHealthBar(200, 560, combatResolver.attacker->hp,
                combatResolver.attacker->maxHp, combatResolver.attacker->is_ally);
    }
    else
    {
        RenderHealthBarDamage(200, 560, combatResolver.attacker->hp,
                combatResolver.attacker->maxHp, combatResolver.damageToAttacker,
                combatResolver.attacker->is_ally);
    }
    if(combatResolver.inc < (float)combatResolver.framesActive * 0.3333)
    {
        RenderHealthBar(400, 560, combatResolver.victim->hp,
                combatResolver.victim->maxHp, combatResolver.victim->is_ally);
    }
    else
    {
        RenderHealthBarDamage(400, 560, combatResolver.victim->hp,
                combatResolver.victim->maxHp, combatResolver.damageToVictim, 
                combatResolver.victim->is_ally);
    }
    */
}

#endif
