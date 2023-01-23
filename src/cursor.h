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

    Animation *animation;
    position animation_offset = position(0, 0);
    direction animation_dir = direction(0, 0);

    Cursor(Spritesheet sheet_in)
    : sheet(sheet_in)
    {}

    void
    Update()
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

        sheet.Update();
        if(!WithinViewport(pos)) // Catch-all
        {
            MoveViewport(pos);
        }
    }

    void
    MoveTo(const position &pos_in, const direction &dir_in)
    {
        pos = pos_in;
        animation_dir = dir_in;
        animation = GetAnimation(MOVE_ANIMATION);
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
