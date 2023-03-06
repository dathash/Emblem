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

// Renders a decal to the screen, given its game coords and spritesheet.
void
RenderTileDecal(const position &pos, const Spritesheet &sheet, double rotation = 0.0)
{
    SDL_Rect destination = {pos.col * TILE_SIZE + X_OFFSET,
                            pos.row * TILE_SIZE + Y_OFFSET,
                            TILE_SIZE, TILE_SIZE};
    SDL_Rect source = {sheet.frame * sheet.size,
                       sheet.track * sheet.size,
                       sheet.size, sheet.size};
    SDL_RenderCopyEx(GlobalRenderer, sheet.texture.sdl_texture, &source, &destination,
                     rotation, NULL, SDL_FLIP_NONE);
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

// Renders an effect sprite to the screen, given its game coords and spritesheet.
void
RenderEffectSprite(EffectType type, const position &pos, const Icons &icons)
{
    const Spritesheet *sheet;
    switch(type)
    {
        case EFFECT_NONE:
        {
            return;
        } break;
        case EFFECT_AFLAME:
        {
            sheet = &icons.aflame;
        } break;
        case EFFECT_PARALYZED:
        {
            sheet = &icons.para;
        } break;
        case EFFECT_SWIFT:
        {
            sheet = &icons.swift;
        } break;
        case EFFECT_STONE:
        {
            sheet = &icons.stone;
        } break;
    }

    float mult = EFFECT_SCALE;
    SDL_Rect destination = {pos.col * TILE_SIZE + X_OFFSET,
                            pos.row * TILE_SIZE + Y_OFFSET + (TILE_SIZE / 2),
                            (int)(sheet->size * mult), 
                            (int)(sheet->size * mult)};
    SDL_Rect source = {sheet->frame * sheet->size,
                       sheet->track * sheet->size,
                       sheet->size, sheet->size};

    SDL_RenderCopy(GlobalRenderer, sheet->texture.sdl_texture, 
                     &source, &destination);
}

// Assumes initial orientation to the right.
// Returns an angle for the sake of SDL_RenderCopyEx
// For use with Visualization components
double
Orient(const direction &dir)
{
    if(dir.row) return 90.0; // (0, 1) or (0, -1)

    return 0.0;
}

SDL_RendererFlip
OrientFlip(const direction &dir)
{
    if(dir.col < 0 || dir.row < 0) return SDL_FLIP_HORIZONTAL; // (-1, 0) or (0, -1)
    return SDL_FLIP_NONE;
}

// Renders a decal inbetween two squares with a direction.
// Generally used for arrows.
void
RenderBetween(const position &from, const position &to, const Spritesheet &sheet)
{
    direction dir = GetDirection(from, to);

    float mult = VIS_SCALE;
                                  // Base value                         Sizing offset (centering)              Arrow direction offset
    SDL_Rect destination = {(int)((from.col * TILE_SIZE + X_OFFSET) - ((TILE_SIZE * mult - TILE_SIZE) / 2)) + (dir.col * TILE_SIZE / 2),
                            (int)((from.row * TILE_SIZE + Y_OFFSET) - ((TILE_SIZE * mult - TILE_SIZE) / 2)) + (dir.row * TILE_SIZE / 2),
                            (int)(TILE_SIZE * mult), 
                            (int)(TILE_SIZE * mult)};
    SDL_Rect source = {sheet.frame * sheet.size,
                       sheet.track * sheet.size,
                       sheet.size, sheet.size};

    SDL_RenderCopyEx(GlobalRenderer, sheet.texture.sdl_texture, 
                     &source, &destination,
                     Orient(dir),
                     NULL, 
                     OrientFlip(dir));
}

// Renders the red arrows which display what an attack will look like.
void
RenderAttackTrajectory(const Tilemap &map, EquipmentType type, const position &from, const position &to, const Icons &icons)
{
    direction dir = GetDirection(from, to);
    switch(type)
    {
        case EQUIP_PUNCH:
        {
            RenderBetween(from, from + GetDirection(from, to), icons.redarrow);
        } break;

        case EQUIP_LINE_SHOT:
        {
            position target = GetFirstTarget(map, from, dir);
            int dist = Distance(from, target);
            for(int i = 1; i < dist; ++i) {
                RenderTileDecal(from + (dir * i), icons.dot, Orient(dir));
            }
            RenderBetween(target - dir, target, icons.redarrow);
        } break;

        case EQUIP_ARTILLERY:
        case EQUIP_LEAP:
        {
        } break;

        default:
        {
            cout << "No visualization of trajectory yet!\n";
        } break;
    }
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


// Renders a Health Bar (For use on map)
void
RenderHealthBar(const position &p, int hp, int maxHp, int damage = 0)
{
    float ratio = (float)hp / maxHp;
    float damage_ratio = min((float)damage / maxHp, ratio);

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
    SDL_Rect damage_rect = {
                         X_OFFSET + p.col * TILE_SIZE + (TILE_SIZE - wide) / 2 + (health_rect.w - (int)(wide * damage_ratio)),
                         Y_OFFSET + p.row * TILE_SIZE + (TILE_SIZE - tall),
                         (int)(wide * damage_ratio), tall
                        };

    SDL_SetRenderDrawColor(GlobalRenderer, gray.r, gray.g, gray.b, gray.a);
    SDL_RenderFillRect(GlobalRenderer, &bar_rect);

    SDL_SetRenderDrawColor(GlobalRenderer, yellow.r, yellow.g, yellow.b, yellow.a);
    SDL_RenderFillRect(GlobalRenderer, &health_rect);

    float val = Lerp(0.0f, 1.0f, sin((GlobalFrame % 60 < 30 ? 60 - (float)(GlobalFrame % 60) : (float)(GlobalFrame % 60)) / 60.0f));
    SDL_SetRenderDrawColor(GlobalRenderer, darkgray.r, darkgray.g, darkgray.b, val * 255);
    SDL_RenderFillRect(GlobalRenderer, &damage_rect);

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

// Confusing struct which contains a unit's outcome from an incident.
struct UnitOutcome
{
    Unit *unit;
    int damage;
    position end_pos;
};


// Helper function to add damage to the pseudo-map structure known as a tracker.
void AddDamage(vector<UnitOutcome> &tracker, Unit *ptr, int damage)
{
    for(UnitOutcome &outcome : tracker)
    {
        if(ptr == outcome.unit)
        {
            outcome.damage += damage;
            return;
        }
    }
    tracker.push_back({ptr, damage, ptr->pos});
    return;
}

// Helper function to move a unit in the pseudo-map structure known as a tracker.
void MoveUnit(vector<UnitOutcome> &tracker, Unit *ptr, position pos)
{
    if(!IsValid(pos)) return;

    for(UnitOutcome &outcome : tracker)
    {
        if(ptr == outcome.unit)
        {
            outcome.end_pos = pos;
            return;
        }
    }
    tracker.push_back({ptr, 0, pos});
    return;
}


// Renders just the zones which an attack will affect.
// TODO: Show push damage ranges, move stuff, etc.
void
RenderAttackZone(const Tilemap &map,
                 const Incident &attack,
                 const Spritesheet &sheet)
{
    const Equip &weapon = *attack.weapon;
    position source = attack.unit->pos;
    position target = attack.unit->pos + attack.offset;
    direction dir = GetDirection(source, target);
    switch(weapon.type)
    {
        case EQUIP_PUNCH:
        {
            for(int i = 1; i <= attack.unit->primary->max_range; ++i) {
                position danger = source + (dir * i);
                RenderTileDecal(danger, sheet);
            }
        } break;
        case EQUIP_LINE_SHOT:
        {
            position danger = GetFirstTarget(map, source, dir);
            RenderTileDecal(danger, sheet);
        } break;
        case EQUIP_ARTILLERY:
        {
            position danger = target;

            // TODO: More than just one square of danger?
            RenderTileDecal(danger, sheet);
        } break;
        default: cout << "AI shouldn't have this kind of attack. " << attack.weapon->type << "\n";
    }
}


// Renders an attack's results on the map.
void
RenderAttack(const Tilemap &map,
             const Incident &attack,
             const Icons &icons)
{
    // This is the main struct that does the tracking of what health bars will render at the end.
    vector<UnitOutcome> tracker = {};

    const Equip &weapon = *attack.weapon;
    vector<position> damaging = {};
    position source = attack.unit->pos;
    position target = attack.unit->pos + attack.offset;
    direction dir = GetDirection(source, target);


    // ===================== plain damage stuff ======================
    switch(weapon.type)
    {
        case EQUIP_PUNCH:
        {
            for(int i = 1; i <= attack.unit->primary->max_range; ++i) {
                damaging.push_back(source + (dir * i));
            }
        } break;
        case EQUIP_LINE_SHOT:
        {
            target = GetFirstTarget(map, source, dir);
            damaging.push_back(target);
        } break;
        case EQUIP_ARTILLERY:
        {
            damaging.push_back(target); // TODO: More than just one square of damage?
        } break;

        default: cout << "AI shouldn't have this kind of attack. " << attack.unit->primary->type << "\n";
    }
    for(const position &pos : damaging)
    {
        Unit *victim = map.tiles[pos.col][pos.row].occupant;
        if(victim) {
            AddDamage(tracker, victim, weapon.damage);
        }
    }

    // ===================== push stuff ======================
    switch(weapon.push)
    {
        case PUSH_AWAY:
        {
            Unit *pushed = map.tiles[target.col][target.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = target + dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

        } break;
        case PUSH_TOWARDS:
        {
            Unit *pushed = map.tiles[target.col][target.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed)
                {
                    // bonk check
                    position push_result = target - dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

        } break;
        case PUSH_TOWARDS_AND_AWAY:
        {
            position t1 = target + dir;
            Unit *pushed = map.tiles[t1.col][t1.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t1 + dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

            position t2 = target - dir;
            pushed = map.tiles[t2.col][t2.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t2 - dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

        } break;

        case PUSH_PERPENDICULAR:
        {
            direction perp = GetPerpendicular(dir);

            position t1 = target + perp;
            Unit *pushed = map.tiles[t1.col][t1.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t1 + perp;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

            position t2 = target - perp;
            pushed = map.tiles[t2.col][t2.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t2 - perp;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }
        } break;

        case PUSH_ALL:
        {
            position t1 = target + dir;
            Unit *pushed = map.tiles[t1.col][t1.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t1 + dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

            position t2 = target - dir;
            pushed = map.tiles[t2.col][t2.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t2 - dir;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

            // perpendicular half
            direction perp = GetPerpendicular(dir);

            position t3 = target + perp;
            pushed = map.tiles[t3.col][t3.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed) {
                    // bonk check
                    position push_result = t3 + perp;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

            position t4 = target - perp;
            pushed = map.tiles[t4.col][t4.row].occupant;
            if(pushed) {
                AddDamage(tracker, pushed, weapon.push_damage);

                if(!pushed->fixed)
                {
                    // bonk check
                    position push_result = t4 - perp;
                    if(IsValid(push_result)) {
                        Unit *bonk = map.tiles[push_result.col][push_result.row].occupant;
                        if(bonk) {
                            AddDamage(tracker, pushed, 1);
                            AddDamage(tracker, bonk, 1);
                        }
                        else {
                            MoveUnit(tracker, pushed, push_result);
                        }
                    }
                }
            }

        } break;

        default:
        {
            //cout << "No visualization of push yet.\n";
        } break;
    }

    // ===================== move stuff ======================
    switch(weapon.move)
    {
        case MOVEMENT_BACKONE:
        {
            AddDamage(tracker, attack.unit, weapon.self_damage);

            // bonk check
            position move_result = source - dir;
            Unit *bonk = map.tiles[move_result.col][move_result.row].occupant;
            if(bonk) {
                AddDamage(tracker, attack.unit, 1);
                AddDamage(tracker, bonk, 1);
            }
            else {
                MoveUnit(tracker, attack.unit, move_result);
            }
        } break;
        case MOVEMENT_RAM:
        {
            AddDamage(tracker, attack.unit, weapon.self_damage);

            position hit = GetFirstTarget(map, source, dir);
            if(map.tiles[hit.col][hit.row].occupant)
            {
                position move_result = hit - dir;
                MoveUnit(tracker, attack.unit, move_result);
            }
            else
            {
                position move_result = hit;
                MoveUnit(tracker, attack.unit, move_result);
            }

            // NOTE: bonking with ram is not possible, I believe?
            // We don't treat the ram as bonking, just rolling right up to something.
            // This might be the wrong abstraction, but for now we're chilling.
        } break;
        case MOVEMENT_LEAP:
        {
            AddDamage(tracker, attack.unit, weapon.self_damage);

            position move_result = target;
            MoveUnit(tracker, attack.unit, move_result);

            // Ditto above comment.
        } break;
        default:
        {
            //cout << "No visualization of this move yet.\n";
        } break;
    }
    // =========================================== Render Visuals ==============
    for(const UnitOutcome &outcome : tracker)
    {
        if(outcome.end_pos != outcome.unit->pos) {
            RenderBetween(outcome.unit->pos, outcome.end_pos, icons.arrow);

            SDL_SetTextureColorMod(outcome.unit->sheet.texture.sdl_texture, lightgray.r, lightgray.g, lightgray.b);
            RenderSprite(outcome.end_pos, outcome.unit->sheet, outcome.unit->IsAlly());

            SDL_SetTextureColorMod(outcome.unit->sheet.texture.sdl_texture, 255, 255, 255);
            RenderSprite(outcome.unit->pos, outcome.unit->sheet, outcome.unit->IsAlly());
        }


        assert(IsValid(outcome.unit->pos));
        RenderHealthBar(outcome.unit->pos, outcome.unit->health, outcome.unit->max_health, outcome.damage);


        // Effects (optional)
        RenderEffectSprite(weapon.effect, target, icons);
    }
}


// Displays the turn order of enemies on the map.
void
RenderQueueOrder(const Tilemap &map, 
                 const Resolution &resolution)
{
    // NOTE: We pull attacks off the back of the queue.
    for(int i = 0; i < resolution.incidents.size(); ++i) {
        RenderTileText(to_string(i + 1),
                       resolution.incidents[resolution.incidents.size() - 1 - i].unit->pos,
                       false,
                       {30, 30},
                       yellow);
    }
}

// Renders the scene from the given game state.
void
Render(const Level &level,
       const vector<position> &rising,
       const Cursor &cursor, 
       const Menu &gameMenu,
       const Resolution &resolution,
       const Icons &icons
      )
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
        RenderTileDecal(cell, icons.rising);
        RenderTileText("rising", cell, true, {4, 30}, yellow);
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
            RenderTileColor(cell, green, OverlayAlphaMod);

        // Debug AI Decision-making system
        /*
        vector<Choice> choices = GetChoicesOrthogonal(*cursor.selected, level.map);
        for(const Choice &choice : choices) {
            RenderTileText(to_string(choice.action_score), 
                           choice.action.move, false, {5, 0}, lightred);
        }
        for(const Choice &choice : choices) {
            RenderTileText(to_string(choice.location_score), 
                           choice.action.move, false, {35, 0}, green);
        }
        */
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

// ================================ ai visualization  ==========================
    if(GlobalAIState == AI_SELECTED)
    {
        for(const position &cell : level.map.accessible)
            RenderTileColor(cell, lightorange, OverlayAlphaMod);
    }

// ========================== Attacking visualization I ========================
    for(const Incident &incident : resolution.incidents)
    {
        if(incident.type == INCIDENT_ATTACK)
        {
            RenderAttackZone(level.map, incident, icons.warning);
        }
        else
        {
            ; // TODO: Render Environment effects
        }
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

                // effects
                RenderEffectSprite(tileToRender.occupant->passive.type, screen_pos, icons);
                for(const Effect &effect : tileToRender.occupant->effects)
                    RenderEffectSprite(effect.type, screen_pos, icons);
            }
        }
    }

    if(GlobalInterfaceState == SELECTED)
        RenderHealthBar(cursor.selected->pos, cursor.selected->health, cursor.selected->max_health);

    if(GlobalInterfaceState == ENEMY_RANGE)
        RenderHealthBar(cursor.selected->pos, cursor.selected->health, cursor.selected->max_health);

// ========================== Attacking visualization ======================
    for(const Incident &incident : resolution.incidents)
    {
        if(incident.type == INCIDENT_ATTACK)
        {
            if(GlobalInterfaceState == ENEMY_RANGE &&
               incident.unit->pos == cursor.selected->pos)
            {
                RenderAttack(level.map, incident, icons);
            }
            RenderAttackTrajectory(level.map, incident.weapon->type, incident.unit->pos, incident.unit->pos + incident.offset, icons);
        }
        else
        {
            ; // TODO: Render Environment effects
        }
    }

    if(GlobalInterfaceState == ATTACK_TARGETING)
    {
        Incident vis = {cursor.selected, cursor.with, cursor.targeting - cursor.selected->pos, INCIDENT_ATTACK};
        RenderAttack(level.map, vis, icons);
        RenderAttackTrajectory(level.map, cursor.with->type, cursor.selected->pos, cursor.targeting, icons);
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
