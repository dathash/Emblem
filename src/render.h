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
    SDL_assert(pos.col >= 0 && pos.row >= 0);
    SDL_Rect tileRect = {TILE_SIZE / 16 + pos.col * TILE_SIZE + X_OFFSET,
                         TILE_SIZE / 16 + pos.row * TILE_SIZE + Y_OFFSET,
                         TILE_SIZE - (TILE_SIZE / 16) * 2, 
                         TILE_SIZE - (TILE_SIZE / 16) * 2};

    SDL_SetRenderDrawColor(GlobalRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(GlobalRenderer, &tileRect);
    SDL_SetRenderDrawColor(GlobalRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(GlobalRenderer, &tileRect);
}

void
RenderAttack(const Tilemap &map,
             const Attack &attack)
{
    switch(attack.unit->primary->type)
    {
        case EQUIP_NONE:
        {
            cout << "WARNING: RenderAttack has NONE attack type.\n";
            RenderTileColor(attack.unit->pos + attack.offset, yellow);
        } break;
        case EQUIP_PUNCH:
        {
            RenderTileColor(attack.unit->pos + attack.offset, attackColor);
        } break;
        case EQUIP_LINE_SHOT:
        {
            position result = GetFirstTarget(map, attack.unit->pos, attack.offset);
            if(result != position(-1, -1))
                RenderTileColor(result, attackColor);
        } break;
        case EQUIP_ARTILLERY:
        {
            RenderTileColor(attack.unit->pos + attack.offset, attackColor);
        } break;

        // Not likely...
        case EQUIP_SELF_TARGET:
        {
            cout << "NO VIS FOR SELF TARGET\n";
        } break;
        case EQUIP_LEAP:
        {
            cout << "NO VIS FOR LEAP\n";
        } break;
        case EQUIP_LASER:
        {
            cout << "NO VIS FOR LASER\n";
        } break;

        default:
        {
        } break;
    }
}

// Renders an individual tile to the screen, given its game coords and tile (for texture).
void
RenderTileTexture(const Tilemap &map, const Tile &tile,
                  const position &pos)
{
    SDL_Rect destination = {pos.col * TILE_SIZE + X_OFFSET,
                            pos.row * TILE_SIZE + Y_OFFSET,
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {tile.atlas_index.col * map.atlas_tile_size, 
                       tile.atlas_index.row * map.atlas_tile_size, 
                       map.atlas_tile_size, map.atlas_tile_size};
    SDL_RenderCopy(GlobalRenderer, map.atlas.sdl_texture, &source, &destination);
}

// Sets color modifiers based on the unit's properties.
void
SetSpriteModifiers(Unit *unit)
{
    if(unit->is_exhausted)
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, exhaustedMod.r, exhaustedMod.g, exhaustedMod.b);
    else
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, readyMod.r, readyMod.g, readyMod.b);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(const position &pos, const Spritesheet &sheet, bool flipped = false)
{
    SDL_Rect destination = {pos.col * TILE_SIZE + X_OFFSET,
                            pos.row * TILE_SIZE + Y_OFFSET,
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.frame * sheet.size,
                       sheet.track * sheet.size,
                       sheet.size, sheet.size};

    SDL_RenderCopyEx(GlobalRenderer, sheet.texture.sdl_texture, 
                     &source, &destination,
                     0, NULL, 
                     (const SDL_RendererFlip)flipped);
}

/*
// Renders a unit's portrait.
void
RenderPortrait(int x, int y, const Unit &unit, bool flipped)
{
    const Texture *portrait;
    switch(expression)
    {
        case EXPR_NEUTRAL: portrait = &unit.neutral; break;
        case EXPR_HAPPY: portrait = &unit.happy; break;
        case EXPR_ANGRY: portrait = &unit.angry; break;
        case EXPR_WINCE: portrait = &unit.wince; break;
        default: SDL_assert(!"ERROR RenderPortrait: Invalid expression."); break;
    }
    SDL_Rect destination = {x, y, 
                            PORTRAIT_SIZE,
                            PORTRAIT_SIZE};
    SDL_Rect source = {0, 0, portrait->width, portrait->height};

    SDL_RenderCopyEx(GlobalRenderer, portrait->sdl_texture, &source, &destination, 
                     0, NULL, (const SDL_RendererFlip)flipped);
}
*/

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
    SDL_Color healthColor = healthBarColor;

    int wide = (int)(TILE_SIZE * 0.8);
    int tall = (int)(TILE_SIZE * 0.1);
    SDL_Rect bar_rect = {
                         X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2,
                         Y_OFFSET + p.row * TILE_SIZE,
                         wide, tall
                        };
    SDL_Rect health_rect = {
                         X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2,
                         Y_OFFSET + p.row * TILE_SIZE,
                         (int)(wide * ratio), tall
                        };

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
Render(const Tilemap &map, 
       const Cursor &cursor, 
       const Menu &gameMenu,
       const Resolution &resolution)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(GlobalRenderer);

    if(GlobalInterfaceState == TITLE_SCREEN)
        return;

// ================================= render map tiles ============================================
    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            position screen_pos = {col, row};
            const Tile &tile = map.tiles[col][row];
            RenderTileTexture(map, tile, screen_pos);
        }
    }

// ================================= render selected or targeted =====================================
    if(GlobalInterfaceState == SELECTED)
    {
        for(const position &cell : map.accessible)
            RenderTileColor(cell, moveColor);

        for(const position &p : cursor.path_draw)
            RenderTileColor(p, pathColor);
    }

    if(GlobalInterfaceState == ENEMY_RANGE)
    {
        for(const position &cell : map.accessible)
            RenderTileColor(cell, aiMoveColor);
    }

    if(GlobalInterfaceState == ATTACK_TARGETING ||
       GlobalInterfaceState == ATTACK_THINKING
      )
    {
        for(const position &cell : map.range)
            RenderTileColor(cell, attackColor);
        for(const position &cell : map.attackable)
            RenderTileColor(cell, yellow);
    }

    if(cursor.selected)
        RenderTileColor(cursor.selected->pos, healColor);

    for(const Attack &attack : resolution.attacks)
    {
        RenderAttack(map, attack);
    }

// ================================ ai visualization  =============================
    if(GlobalAIState == AI_SELECTED)
    {
        for(const position &cell : map.accessible)
            RenderTileColor(cell, aiMoveColor);
    }

// ================================= render sprites ================================================
    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            const Tile &tileToRender = map.tiles[col][row];
            if(tileToRender.occupant)
            {
                position screen_pos = {col , row};

                SetSpriteModifiers(tileToRender.occupant);

                RenderSprite(screen_pos, tileToRender.occupant->sheet, tileToRender.occupant->IsAlly());
                RenderHealthBarSmall(screen_pos, tileToRender.occupant->health, tileToRender.occupant->max_health);
            }
        }
    }

// ================================= render cursor ================================================
    if(
        GlobalInterfaceState == ENEMY_RANGE ||
        GlobalInterfaceState == NEUTRAL_OVER_GROUND ||
        GlobalInterfaceState == NEUTRAL_OVER_ENEMY ||
        GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
        GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
        GlobalInterfaceState == SELECTED ||
        GlobalInterfaceState == ATTACK_THINKING ||
        GlobalInterfaceState == ATTACK_TARGETING
      )
    {
        RenderSprite(cursor.pos, cursor.sheet);
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

    /*
    // Portraits
    if(GlobalInterfaceState == UNIT_INFO ||
	   GlobalInterfaceState == ENEMY_INFO)
    {
        SDL_Rect bg_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, yellow.r, yellow.g, yellow.b, 150);
        SDL_RenderFillRect(GlobalRenderer, &bg_rect);

        const Unit *subject = map.tiles[cursor.pos.col][cursor.pos.row].occupant;
        SDL_assert(subject);
        int x_pos = 480;
        if(subject->is_ally)
            x_pos = -50;

        RenderPortrait(x_pos, 0, *subject,
                       subject->health < subject->MaxHealth() * 0.5 ? EXPR_WINCE : EXPR_NEUTRAL,
                       subject->is_ally);
    }
    */
}

#endif
