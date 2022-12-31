// Author: Alex Hartford
// Program: Emblem
// File: Render
// Date: July 2022

#ifndef RENDER_H
#define RENDER_H

// ================================ Rendering ==========================================
// Renders an individual tile to the screen, given its game coords and color.
// TODO: Add a cool outline? Dynamic tiles like in GBA FE?
void
RenderTileColor(int col, int row, const SDL_Color &color)
{
    //TODO: Put the viewport stuff all in here, so the rest of the
    // code doesn't have to worry about it.

    //assert(col >= 0 && row >= 0);
    SDL_Rect tileRect = {TILE_SIZE / 16 + col * TILE_SIZE, TILE_SIZE / 16 + row * TILE_SIZE, 
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
                  int col, int row)
{
    SDL_Rect destination = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {tile.atlas_index.first * map.atlas_tile_size, 
                       tile.atlas_index.second * map.atlas_tile_size, 
                       map.atlas_tile_size, map.atlas_tile_size};
    SDL_RenderCopy(GlobalRenderer, map.atlas.sdlTexture, &source, &destination);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(int col, int row, const SpriteSheet &sheet)
{
    SDL_Rect destination = {col * TILE_SIZE, 
                            row * TILE_SIZE, 
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.frame * sheet.size, sheet.track * sheet.size, sheet.size, sheet.size};

    SDL_RenderCopy(GlobalRenderer, sheet.texture.sdlTexture, &source, &destination);
}

// Renders a unit's portrait.
void
RenderPortrait(int x, int y, const Texture &portrait)
{
    SDL_Rect destination = {x, y, 
                            PORTRAIT_SIZE,
                            PORTRAIT_SIZE};
    SDL_Rect source = {0, 0, portrait.width, portrait.height};

    SDL_RenderCopy(GlobalRenderer, portrait.sdlTexture, &source, &destination);
}

// Renders a given texture at a pixel point.
// (Meant for debug text)
void
RenderText(const Texture &texture, int x, int y)
{
    SDL_Rect destination = {x, y, texture.width, texture.height};
    SDL_RenderCopy(GlobalRenderer, texture.sdlTexture, NULL, &destination);
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
        SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
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
        SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &healthRect);
    }
}


// Renders the scene from the given game state.
void
Render(const Tilemap &map, const Cursor &cursor, 
       const Menu &gameMenu, const Menu &unitMenu,
       const CombatResolver &combatResolver)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(GlobalRenderer);

// ================================= render map tiles ============================================
    for(int col = viewportCol; col < VIEWPORT_WIDTH + viewportCol; ++col)
    {
        for(int row = viewportRow; row < VIEWPORT_HEIGHT + viewportRow; ++row)
        {
            int screenCol = col - viewportCol;
            int screenRow = row - viewportRow;
            const Tile &tile = map.tiles[col][row];
            RenderTileTexture(map, tile, screenCol, screenRow);
        }
    }

// ================================= render selected or targeted =====================================
    if(GlobalInterfaceState == SELECTED_OVER_GROUND ||
       GlobalInterfaceState == SELECTED_OVER_INACCESSIBLE ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY)
    {
        for(pair<int, int> cell : map.accessible)
        {
            if(cursor.WithinViewport(cell.first, cell.second))
            {
                RenderTileColor(cell.first - viewportRow, 
                           cell.second - viewportRow, 
                           moveColor);
            }
        }

        for(const point &p : cursor.path_draw)
        {
            RenderTileColor(p.first - viewportRow, 
                       p.second - viewportRow, 
                       pathColor);
        }
    }

    if(GlobalInterfaceState == ENEMY_INFO)
    {
        for(pair<int, int> cell : map.accessible)
        {
            if(cursor.WithinViewport(cell.first, cell.second))
            {
                RenderTileColor(cell.first - viewportRow, 
                           cell.second - viewportRow, 
                           aiMoveColor);
            }
        }
    }

    if(GlobalInterfaceState == ATTACK_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == ATTACK_TARGETING_OVER_UNTARGETABLE)
    {
        for(pair<int, int> cell : map.interactible)
        {
            if(cursor.WithinViewport(cell.first, cell.second))
            {
                RenderTileColor(cell.first - viewportRow, 
                           cell.second - viewportRow, 
                           attackColor);
            }
        }
    }

    if(GlobalInterfaceState == HEALING_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == HEALING_TARGETING_OVER_UNTARGETABLE)
    {
        for(pair<int, int> cell : map.interactible)
        {
            if(cursor.WithinViewport(cell.first, cell.second))
            {
                RenderTileColor(cell.first - viewportRow, 
                           cell.second - viewportRow, 
                           healColor);
            }
        }
    }

// ================================ ai visualization  =============================
    if(GlobalAIState == SELECTED)
    {
        for(pair<int, int> cell : map.accessible)
        {
            if(cursor.WithinViewport(cell.first, cell.second))
            {
                RenderTileColor(cell.first - viewportRow, 
                           cell.second - viewportRow, 
                           aiMoveColor);
            }
        }
    }

// ================================= render sprites ================================================
    for(int col = viewportCol; col < VIEWPORT_WIDTH + viewportCol; ++col)
    {
        for(int row = viewportRow; row < VIEWPORT_HEIGHT + viewportRow; ++row)
        {
            int screenCol = col - viewportCol;
            int screenRow = row - viewportRow;
            const Tile &tileToRender = map.tiles[col][row];
            if(tileToRender.occupant)
            {
                if(tileToRender.occupant->isExhausted)
                {
                    SDL_SetTextureColorMod(tileToRender.occupant->sheet.texture.sdlTexture, exhaustedMod.r, exhaustedMod.g, exhaustedMod.b);
                }
                else
                {
                    SDL_SetTextureColorMod(tileToRender.occupant->sheet.texture.sdlTexture, readyMod.r, readyMod.g, readyMod.b);
                }
                RenderSprite(screenCol, screenRow, tileToRender.occupant->sheet);
            }
        }
    }


// ================================= render cursor ================================================
    RenderSprite(cursor.col - viewportCol, cursor.row - viewportRow, cursor.sheet);


// ==================================== menus =====================================================
    // Game Menu
    if(GlobalInterfaceState == GAME_MENU_ROOT)
    {
        for(int i = 0; i < gameMenu.rows; ++i)
        {
            SDL_Rect menuRect = {TILE_SIZE * VIEWPORT_WIDTH, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(gameMenu.optionTextTextures[i], menuRect.x, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {TILE_SIZE * VIEWPORT_WIDTH, MENU_ROW_HEIGHT * gameMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
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
            SDL_Rect menuRect = {TILE_SIZE * VIEWPORT_WIDTH, i * MENU_ROW_HEIGHT, MENU_WIDTH, MENU_ROW_HEIGHT};

            SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);
            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(unitMenu.optionTextTextures[i], menuRect.x, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {TILE_SIZE * VIEWPORT_WIDTH, MENU_ROW_HEIGHT * unitMenu.current, MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, uiTextColor.r, uiTextColor.g, uiTextColor.b, uiTextColor.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    // Portraits
    if(GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
       GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
       GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
       GlobalInterfaceState == SELECTED_OVER_ALLY ||
       GlobalInterfaceState == SELECTED_OVER_ENEMY ||
       GlobalInterfaceState == ATTACK_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == HEALING_TARGETING_OVER_TARGET ||
       GlobalInterfaceState == UNIT_MENU_ROOT ||
	   GlobalInterfaceState == UNIT_INFO ||
	   GlobalInterfaceState == ENEMY_INFO ||
	   GlobalInterfaceState == PREVIEW_ATTACK ||
	   GlobalInterfaceState == PREVIEW_HEALING)
    {
        // TODO: Broken because level loading can put us into degenerate gamestates
        assert(map.tiles[cursor.col][cursor.row].occupant);
        RenderPortrait(500, 400, map.tiles[cursor.col][cursor.row].occupant->portrait);
    }

	// Combat Screen
	if(GlobalResolvingAction)
	{
        if(combatResolver.inc < (float)combatResolver.framesActive * 0.6666)
        {
            RenderHealthBar(0, 560, combatResolver.attacker->hp,
                    combatResolver.attacker->maxHp, combatResolver.attacker->isAlly);
        }
        else
        {
            RenderHealthBarDamage(0, 560, combatResolver.attacker->hp,
                    combatResolver.attacker->maxHp, combatResolver.damageToAttacker,
                    combatResolver.attacker->isAlly);
        }
        if(combatResolver.inc < (float)combatResolver.framesActive * 0.3333)
        {
            RenderHealthBar(200, 560, combatResolver.victim->hp,
                    combatResolver.victim->maxHp, combatResolver.victim->isAlly);
        }
        else
        {
            RenderHealthBarDamage(200, 560, combatResolver.victim->hp,
                    combatResolver.victim->maxHp, combatResolver.damageToVictim, 
                    combatResolver.victim->isAlly);
        }
	}
}

#endif
