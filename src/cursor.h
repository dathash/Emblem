// Author: Alex Hartford
// Program: Emblem
// File: Cursor

#ifndef CURSOR_H
#define CURSOR_H

struct Cursor
{
    position pos = {-1, -1};
    Unit *selected = nullptr;

    position targeting = {-1, -1};

    Spritesheet sheet;
    path path_draw = {};


    Cursor(Spritesheet sheet_in)
    : sheet(sheet_in)
    {}

    void
    Update(Tilemap *map)
    {
        sheet.Update();
    }


    void
    MoveTo(const position &pos_in, const direction &dir_in)
    {
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
