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

// Sets color modifiers based on the unit's properties.
void
SetSpriteModifiers(Unit *unit)
{
    if(unit->is_exhausted)
    {
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, exhaustedMod.r, exhaustedMod.g, exhaustedMod.b);
    }
    else if(unit->buff)
    {
        switch(unit->buff->stat)
        {
            case STAT_ATTACK:
            {
                SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, buffAtkMod.r, buffAtkMod.g, buffAtkMod.b);
            } break;
            case STAT_DEFENSE:
            {
                SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, buffDefMod.r, buffDefMod.g, buffDefMod.b);
            } break;
            case STAT_APTITUDE:
            {
                SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, buffAptMod.r, buffAptMod.g, buffAptMod.b);
            } break;
            case STAT_SPEED:
            {
                SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, buffSpdMod.r, buffSpdMod.g, buffSpdMod.b);
            } break;
            default:
            {
                cout << "WARN Render(): Unit has unhandled buff color mod\n";
            } break;
        }
    }
    else if(unit->is_boss)
    {
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, buffAtkMod.r, buffAtkMod.g, buffAtkMod.b);
    }
    else
    {
        SDL_SetTextureColorMod(unit->sheet.texture.sdl_texture, readyMod.r, readyMod.g, readyMod.b);
    }
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
RenderPortrait(int x, int y, const Unit &unit, 
               Expression expression, bool flipped)
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
RenderHealthBarSmall(const position &p, int hp, int maxHp,
                     const position &animation_offset = position(0, 0))
{
    float ratio = (float)hp / maxHp;
    SDL_Color healthColor = PiecewiseColors(red, yellow, green, ratio);

    SDL_Rect bar_rect = {p.col * TILE_SIZE + 7     + animation_offset.col, 
                            p.row * TILE_SIZE + 50 + animation_offset.row,
                            50, 8};
    SDL_Rect health_rect = {p.col * TILE_SIZE + 7  + animation_offset.col,
                            p.row * TILE_SIZE + 50 + animation_offset.row,
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
       const Menu &conversationMenu,
       const ConversationList &conversations, 
       const Fight &fight, const Fade &level_fade, const Fade &turn_fade,
       const Advancement &advancement)
{
    SDL_SetRenderDrawBlendMode(GlobalRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GlobalRenderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(GlobalRenderer);

    if(GlobalInterfaceState == TITLE_SCREEN)
        return;

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
        for(const position &cell : map.vis_range)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           clearColor);
            }
        }

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
            if(WithinViewport(p))
            {
                RenderTileColor({p.col - viewportCol, 
                           p.row - viewportRow}, 
                           pathColor);
            }
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

        for(const position &cell : map.vis_range)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           clearColor);
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

    if(GlobalInterfaceState == ABILITY_TARGETING)
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

        for(const position &cell : map.ability)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           yellow);
            }
        }
    }

    if(GlobalInterfaceState == TALK_TARGETING)
    {
        for(const position &cell : map.adjacent)
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
    if(GlobalAIState == AI_SELECTED)
    {
        for(const position &cell : map.vis_range)
        {
            if(WithinViewport(cell))
            {
                RenderTileColor({cell.col - viewportCol, 
                           cell.row - viewportRow}, 
                           clearColor);
            }
        }

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
                position screen_pos = {col - viewportCol, row - viewportRow};

                SetSpriteModifiers(tileToRender.occupant);

                RenderSprite(screen_pos, tileToRender.occupant->sheet, tileToRender.occupant->is_ally, tileToRender.occupant->animation_offset);
                RenderHealthBarSmall(screen_pos, tileToRender.occupant->health, tileToRender.occupant->max_health, tileToRender.occupant->animation_offset);
            }
        }
    }

// ================================= render cursor ================================================
    if(
        GlobalInterfaceState == NEUTRAL_OVER_GROUND ||
        GlobalInterfaceState == NEUTRAL_OVER_ENEMY ||
        GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
        GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
        GlobalInterfaceState == SELECTED_OVER_GROUND ||
        GlobalInterfaceState == SELECTED_OVER_INACCESSIBLE ||
        GlobalInterfaceState == SELECTED_OVER_ALLY ||
        GlobalInterfaceState == SELECTED_OVER_ENEMY ||
        GlobalInterfaceState == ATTACK_TARGETING ||
        GlobalInterfaceState == ABILITY_TARGETING ||
        GlobalInterfaceState == TALK_TARGETING ||
        GlobalInterfaceState == ENEMY_INFO ||
        GlobalInterfaceState == ENEMY_RANGE
      )
    {
        if(WithinViewport(cursor.pos))
            RenderSprite(cursor.pos - position(viewportCol, viewportRow), 
                         cursor.sheet, false, cursor.animation_offset);
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

    if(GlobalInterfaceState == CONVERSATION_MENU)
    {
        for(int i = 0; i < conversationMenu.rows; ++i)
        {
            SDL_Rect menuRect = {400, 140 + i * MENU_ROW_HEIGHT, CONV_MENU_WIDTH, MENU_ROW_HEIGHT};

            if(i < conversations.list.size() && conversations.list[i].done)
                SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
            else
                SDL_SetRenderDrawColor(GlobalRenderer, uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            SDL_RenderFillRect(GlobalRenderer, &menuRect);

            SDL_SetRenderDrawColor(GlobalRenderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
            SDL_RenderDrawRect(GlobalRenderer, &menuRect);

            RenderText(conversationMenu.optionTextTextures[i], menuRect.x + 10, menuRect.y);
        }

        SDL_Rect menuSelectorRect = {400, 140 + MENU_ROW_HEIGHT * conversationMenu.current, CONV_MENU_WIDTH, MENU_ROW_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, uiSelectorColor.r, uiSelectorColor.g, uiSelectorColor.b, uiSelectorColor.a);
        SDL_RenderFillRect(GlobalRenderer, &menuSelectorRect);
        SDL_SetRenderDrawColor(GlobalRenderer, black.r, black.g, black.b, black.a);
        SDL_RenderDrawRect(GlobalRenderer, &menuSelectorRect);
    }

    // Portraits
    if(GlobalInterfaceState == UNIT_INFO ||
	   GlobalInterfaceState == ENEMY_INFO)
    {
        const Unit *subject = map.tiles[cursor.pos.col][cursor.pos.row].occupant;
        SDL_assert(subject);
        int x_pos = 480;
        if(subject->is_ally)
            x_pos = -50;

        RenderPortrait(x_pos, 0, *subject,
                       subject->health < subject->max_health * 0.5 ? EXPR_WINCE : EXPR_NEUTRAL,
                       subject->is_ally);
    }

    if(GlobalInterfaceState == PREVIEW_ATTACK ||
	   GlobalInterfaceState == PREVIEW_ABILITY)
    {
        SDL_assert(cursor.selected);

        RenderPortrait(-100, 0, *cursor.selected, 
                       GlobalInterfaceState == PREVIEW_ABILITY ? EXPR_HAPPY : EXPR_ANGRY, 
                       true);
        RenderPortrait(430, 0, *cursor.targeted, 
                       GlobalInterfaceState == PREVIEW_ABILITY ? EXPR_HAPPY : EXPR_ANGRY, 
                       false);
    }

    if(GlobalInterfaceState == RESOLVING_ADVANCEMENT ||
	   GlobalAIState == AI_RESOLVING_ADVANCEMENT)
    {
        RenderPortrait(-100, 0, *advancement.recipient,
                       EXPR_HAPPY,
                       true);
    }

    if(GlobalInterfaceState == CONVERSATION ||
       GlobalInterfaceState == BATTLE_CONVERSATION ||
       GlobalInterfaceState == PRELUDE ||
       GlobalInterfaceState == VILLAGE_CONVERSATION ||
       GlobalInterfaceState == CUTSCENE)
    {
        Conversation conversation;
        if(GlobalInterfaceState == CONVERSATION)
            conversation = conversations.list[conversations.index];

        if(GlobalInterfaceState == BATTLE_CONVERSATION ||
           GlobalInterfaceState == VILLAGE_CONVERSATION)
            conversation = *conversations.current_conversation;

        if(GlobalInterfaceState == PRELUDE)
            conversation = conversations.prelude;

        if(GlobalInterfaceState == CUTSCENE)
            conversation = conversations.current_cutscene->second;

        SDL_Rect bg_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_SetRenderDrawColor(GlobalRenderer, yellow.r, yellow.g, yellow.b, 150);
        SDL_RenderFillRect(GlobalRenderer, &bg_rect);

        if(conversation.active[3])
            RenderPortrait(150, 0, *conversation.four,
                           conversation.expressions[3], true);
        if(conversation.active[2])
            RenderPortrait(350, 0, *conversation.three,
                           conversation.expressions[2], false);

        if(conversation.active[0])
            RenderPortrait(-50, 0, *conversation.one,
                           conversation.expressions[0], true);
        if(conversation.active[1])
            RenderPortrait(500, 0, *conversation.two,
                           conversation.expressions[1], false);

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

    SDL_Rect fade_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    if(level_fade.animation)
    {
        SDL_SetRenderDrawColor(GlobalRenderer, level_fade.color.r, level_fade.color.g, level_fade.color.b, (int)(255 * level_fade.amount));
        SDL_RenderFillRect(GlobalRenderer, &fade_rect);
    }

    if(turn_fade.animation)
    {
        SDL_SetRenderDrawColor(GlobalRenderer, turn_fade.color.r, turn_fade.color.g, turn_fade.color.b, (int)(255 * turn_fade.amount));
        SDL_RenderFillRect(GlobalRenderer, &fade_rect);

        if(turn_fade.amount > 0.3f)
        {
            if(turn_fade.show_first_texture)
                RenderText(turn_fade.texture_one, 400, 200);
            else
                RenderText(turn_fade.texture_two, 400, 200);
            RenderText(turn_fade.turn_count_texture, 450, 230);
        }
    }
}

#endif
