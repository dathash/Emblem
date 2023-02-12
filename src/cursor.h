// Author: Alex Hartford
// Program: Cursor
// File: Structs

#ifndef CURSOR_H
#define CURSOR_H

struct Cursor
{
    position pos = {-1, -1};
    Unit *selected = nullptr;
    position redo = {-1, -1}; // Where the cursor was before placing a unit
    Unit *targeted = nullptr;
    position source = {-1, -1}; // Where the cursor was before choosing a target

    Spritesheet sheet;
    path path_draw = {};

    Animation *animation = nullptr;
    Animation *unit_animation = nullptr;
    position animation_offset = position(0, 0);
    direction animation_dir = direction(0, 0);

    Cursor(Spritesheet sheet_in)
    : sheet(sheet_in)
    {}

    ~Cursor()
    {
        delete animation;
        delete unit_animation;
    }

    void
    Update(Tilemap *map)
    {
        if(animation)
        {
            float value = animation->Value(CHANNEL_ONE);
            position offset = (animation_dir * TILE_SIZE) * value;
            animation_offset = offset;

            if(animation->Update())
            {
                delete animation;
                animation = nullptr;

                animation_offset = {0, 0};
            }
        }

        if(unit_animation)
        {
            float value = unit_animation->Value(CHANNEL_ONE);
            int index = (int)(value * path_draw.size());
            float amount_tween = (float)(value * path_draw.size()) - (int)(value * path_draw.size());
            position between = {0, 0};
            if(index + 1 < path_draw.size())
            {
                between = ((path_draw[index+1] - path_draw[index]) * TILE_SIZE * amount_tween);
                direction dir = GetDirection(path_draw[index+1], path_draw[index]);
                selected->sheet.ChangeTrackMovement(dir);
            }

            selected->animation_offset = 
                (path_draw[index] - selected->pos) * TILE_SIZE
                + between;

            if(unit_animation->Update())
            {
                delete unit_animation;
                unit_animation = nullptr;

                map->tiles[redo.col][redo.row].occupant = nullptr;
                map->tiles[pos.col][pos.row].occupant = selected;

                selected->pos = pos;
                selected->sheet.ChangeTrack(TRACK_ACTIVE);

                selected->animation_offset = {0, 0};

                GlobalInterfaceState = UNIT_MENU_ROOT;
                EmitEvent(PLACE_UNIT_EVENT);
            }
        }

        sheet.Update();

        if(!WithinViewport(pos)) // Catch-all
        {
            MoveViewport(pos);
        }
    }

    void
    MoveTo(const position &pos_in, const direction &dir_in, 
           int map_width, int map_height)
    {
        // Check Soft Viewport
        if(
           (ColWithinSoftViewport(pos) && !ColWithinSoftViewport(pos_in) &&
           (dir_in.col && pos_in.col >= 3 && pos_in.col < map_width - 3))
           ||
           (RowWithinSoftViewport(pos) && !RowWithinSoftViewport(pos_in) &&
            (dir_in.row && pos_in.row >= 2 && pos_in.row < map_height - 2)))
        {
            MoveViewportDirection(dir_in * -1);
        }
        else
        {
            animation_dir = dir_in;
            animation = GetAnimation(MOVE_ANIMATION, 1.0f);
        }

        pos = pos_in;

        EmitEvent(MOVE_CURSOR_EVENT);
    }

    // Places the cursor at a position.
    void
    PlaceAt(const position &pos_in)
    {
        pos = pos_in;
        path_draw = {};
    }
};

#endif
