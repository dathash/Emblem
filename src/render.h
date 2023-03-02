// Author: Alex Hartford
// Program: Emblem
// File: Render

#ifndef RENDER_H
#define RENDER_H

// ================================ Rendering ==========================================
// Renders an individual tile to the screen, given its game coords and color.
void
RenderTileColor(const position &pos, const SDL_Color &color, int alpha = 255)
{
    SDL_assert(pos.col >= 0 && pos.row >= 0);
    SDL_Rect tileRect = {TILE_SIZE / 16 + pos.col * TILE_SIZE + X_OFFSET,
                         TILE_SIZE / 16 + pos.row * TILE_SIZE + Y_OFFSET,
                         TILE_SIZE - (TILE_SIZE / 16) * 2, 
                         TILE_SIZE - (TILE_SIZE / 16) * 2};

    SDL_SetRenderDrawColor(GlobalRenderer, color.r, color.g, color.b, alpha);
    SDL_RenderFillRect(GlobalRenderer, &tileRect);
    SDL_SetRenderDrawColor(GlobalRenderer, white.r, white.g, white.b, 255);
    SDL_RenderDrawRect(GlobalRenderer, &tileRect);
}

void
RenderAttack(const Tilemap &map,
             const Incident &attack,
             const SDL_Color &color = lightred,
             int alpha = 128)
{
    switch(attack.unit->primary->type)
    {
        case EQUIP_NONE:
        {
            cout << "WARNING: RenderAttack has NONE attack type.\n";
            RenderTileColor(attack.unit->pos + attack.offset, color, alpha);
        } break;
        case EQUIP_PUNCH:
        {
            for(int i = 1; i <= attack.unit->primary->max_range; ++i)
                RenderTileColor(attack.unit->pos + (attack.offset * i), color, alpha);
        } break;
        case EQUIP_LINE_SHOT:
        {
            position result = GetFirstTarget(map, attack.unit->pos, attack.offset);
            if(result != position(-1, -1))
                RenderTileColor(result, color, alpha);
        } break;
        case EQUIP_ARTILLERY:
        {
            RenderTileColor(attack.unit->pos + attack.offset, color, alpha);
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
    }
}

position
ToScreenPosition(const position &map_pos, const position &offset = {0, 0})
{
    return {
        map_pos.col * TILE_SIZE + X_OFFSET + offset.col,
        map_pos.row * TILE_SIZE + Y_OFFSET + offset.row
    };
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
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, darkbeige.r, darkbeige.g, darkbeige.b);
    else
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, 255, 255, 255);
}

// Renders a sprite to the screen, given its game coords and spritesheet.
void
RenderSprite(const position &pos, const Spritesheet &sheet, bool flipped = false)
{
    float mult = SPRITE_SCALE;
    SDL_Rect destination = {(int)((pos.col * TILE_SIZE + X_OFFSET) - ((TILE_SIZE * mult - TILE_SIZE) / 2)),
                            (int)((pos.row * TILE_SIZE + Y_OFFSET) - ((TILE_SIZE * mult - TILE_SIZE) / 2)),
                            (int)(TILE_SIZE * mult), 
                            (int)(TILE_SIZE * mult)};
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

void
RenderText(string text, int x, int y, bool small = false, const SDL_Color &color = black)
{
    Texture texture = LoadTextureText(text.c_str(), color, 0, small);
    SDL_Rect destination = {x, y, texture.width, texture.height};
    SDL_RenderCopy(GlobalRenderer, texture.sdl_texture, NULL, &destination);
}

void
RenderTileText(string text, const position &pos,
               bool small = true,
               const position &offset = {0, 0},
               const SDL_Color &color = black)
{
    position screen_pos = ToScreenPosition(pos, offset);
    RenderText(text, screen_pos.col, screen_pos.row, small, color);
}

// Displays the turn order of enemies on the map.
void
RenderQueueOrder(const Tilemap &map, 
                 const Resolution &resolution)
{
    // NOTE: We pull attacks off the back of the queue.
    for(int i = 0; i < resolution.incidents.size(); ++i) {
        RenderTileText(to_string(i + 1),
                       resolution.incidents[resolution.incidents.size() - 1 - i].unit->pos);
    }
}

// Renders a Health Bar (For use on map)
void
RenderHealthBarSmall(const position &p, int hp, int maxHp)
{
    float ratio = (float)hp / maxHp;

    int wide = (int)(TILE_SIZE * 0.8);
    int tall = (int)(TILE_SIZE * 0.1);
    SDL_Rect bar_rect = {
                         X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2,
                         Y_OFFSET + p.row * TILE_SIZE + (TILE_SIZE - tall),
                         wide, tall
                        };
    SDL_Rect health_rect = {
                         X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2,
                         Y_OFFSET + p.row * TILE_SIZE + (TILE_SIZE - tall),
                         (int)(wide * ratio), tall
                        };

    SDL_SetRenderDrawColor(GlobalRenderer, gray.r, gray.g, gray.b, gray.a);
    SDL_RenderFillRect(GlobalRenderer, &bar_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, yellow.r, yellow.g, yellow.b, yellow.a);
    SDL_RenderFillRect(GlobalRenderer, &health_rect);

    for(int i = 0; i < maxHp; ++i)
    {
        int hp_width = (int)(wide / maxHp); // how wide one tick of health is.
        SDL_Rect div_rect = {
                             X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2 + (i * hp_width),
                             Y_OFFSET + p.row * TILE_SIZE + (TILE_SIZE - tall),
                             hp_width, tall
                            };
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &div_rect);
    }
}

// Renders a Health Bar (For Combat Animations)
void
RenderHealthBarCombat(const position &p, int hp, int maxHp)
{
    float ratio = (float)hp / maxHp;

    SDL_Rect bar_rect = {p.col, p.row,
                         200, 24};
    SDL_Rect health_rect = {p.col, p.row,
                            (int)(200 * ratio), 24};

    SDL_SetRenderDrawColor(GlobalRenderer, darkgray.r, darkgray.g, darkgray.b, darkgray.a);
    SDL_RenderFillRect(GlobalRenderer, &bar_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, green.r, green.g, green.b, green.a);
    SDL_RenderFillRect(GlobalRenderer, &health_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
    SDL_RenderDrawRect(GlobalRenderer, &bar_rect);
}

// Renders the scene from the given game state.
void
Render(const Level &level,
       const vector<position> &rising,
       const Cursor &cursor, 
       const Menu &gameMenu,
       const Resolution &resolution)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, beige.r, beige.g, beige.b, beige.a);
    SDL_RenderClear(GlobalRenderer);

    if(GlobalInterfaceState == TITLE_SCREEN)
        return;

// ================================= render map tiles ============================================
    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            position screen_pos = {col, row};
            const Tile &tile = level.map.tiles[col][row];
            RenderTileTexture(level.map, tile, screen_pos);
        }
    }

    for(const position& cell : rising) {
        RenderTileColor(cell, lightorange, OverlayAlphaMod);
        RenderTileText("rising", cell, true, {4, 30}, red);
    }

// ================================= render selected or targeted =====================================
    if(GlobalInterfaceState == SELECTED)
    {
        for(const position &cell : level.map.accessible)
            RenderTileColor(cell, green, OverlayAlphaMod);

        for(const position &p : cursor.path_draw)
            RenderTileColor(p, lightblue, OverlayAlphaMod);
    }

    if(GlobalInterfaceState == ENEMY_RANGE)
    {
        for(const position &cell : level.map.accessible)
            RenderTileColor(cell, lightorange, OverlayAlphaMod);

        vector<Choice> choices = GetChoicesOrthogonal(*cursor.selected, level.map);
        for(const Choice &choice : choices) {
            RenderTileText(to_string(choice.action_score), 
                           choice.action.move, false, {5, 0}, lightred);
        }
        for(const Choice &choice : choices) {
            RenderTileText(to_string(choice.location_score), 
                           choice.action.move, false, {35, 0}, green);
        }
    }

    if(GlobalInterfaceState == ATTACK_TARGETING ||
       GlobalInterfaceState == ATTACK_THINKING
      )
    {
        for(const position &cell : level.map.range)
            RenderTileColor(cell, red, OverlayAlphaMod);
        for(const position &cell : level.map.attackable)
            RenderTileColor(cell, yellow, OverlayAlphaMod);
    }

    if(cursor.selected)
        RenderTileColor(cursor.selected->pos, green, OverlayAlphaMod);

    for(const Incident &incident : resolution.incidents)
    {
        if(incident.type == INCIDENT_ATTACK)
        {
            if(GlobalInterfaceState == ENEMY_RANGE &&
               incident.unit->pos == cursor.selected->pos)
            {
                RenderAttack(level.map, incident, red, 240);
            }
            else
            {
                RenderAttack(level.map, incident, lightred, OverlayAlphaMod);
            }
        }
        else
        {
            ; // TODO: Render Environment effects
        }
    }

// ================================ ai visualization  ==========================
    if(GlobalAIState == AI_SELECTED)
    {
        for(const position &cell : level.map.accessible)
            RenderTileColor(cell, lightorange, OverlayAlphaMod);
    }

// ================================= render sprites ============================
    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            const Tile &tileToRender = level.map.tiles[col][row];
            if(tileToRender.occupant)
            {
                position screen_pos = {col, row};

                SetSpriteModifiers(tileToRender.occupant);

                RenderSprite(screen_pos, tileToRender.occupant->sheet, tileToRender.occupant->IsAlly());
                RenderHealthBarSmall(screen_pos, tileToRender.occupant->health, tileToRender.occupant->max_health);
            }
        }
    }

// ========================================= warping ===========================
    if(GlobalInterfaceState == WARP)
    {
        for(int col = 0; col < MAP_WIDTH; ++col)
        {
            for(int row = 0; row < MAP_HEIGHT; ++row)
            {
                if(Warpable({col, row}) && !level.map.tiles[col][row].occupant)
                    RenderTileColor({col, row}, lightbeige, OverlayAlphaMod);
            }
        }

        if(!level.to_warp.empty())
            RenderSprite(cursor.pos, level.to_warp.back()->sheet, true);
    }

// ================================= render cursor ================================================
    if(
        GlobalInterfaceState == ENEMY_RANGE ||
        GlobalInterfaceState == NEUTRAL ||
        GlobalInterfaceState == SELECTED ||
        GlobalInterfaceState == ATTACK_THINKING ||
        GlobalInterfaceState == ATTACK_TARGETING ||
        GlobalInterfaceState == WARP
      )
    {
        RenderSprite(cursor.pos, cursor.sheet);
    }

// ==================================== queue ==================================
    if(GlobalInterfaceState == QUEUE)
    {
        RenderQueueOrder(level.map, resolution);
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
