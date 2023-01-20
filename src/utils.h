// Author: Alex Hartford
// Program: Emblem
// File: Utils

#ifndef UTILS_H
#define UTILS_H

struct Timer
{
    Uint32 current = 0;
    Uint32 last_frame = 0;
    Uint32 end;
    bool paused = false;

    Timer(int seconds)
    {
        last_frame = SDL_GetTicks();
        end = seconds * 1000;
    }

    Timer()
    {} // NOTE: This is c++ weirdness. I'm sure I could figure it out if I
       // gave it a few minutes.

    bool
    Update()
    {
        if(paused)
            return false;

        int since_last_frame = SDL_GetTicks() - last_frame;

        if(since_last_frame > 30) // Quick hack to fix pausing
            since_last_frame = 30; // TODO: Really fix this someday

        current += since_last_frame;

        last_frame = SDL_GetTicks();
        if(current >= end)
            return true;
        return false;
    }

    void
    Pause()
    {
        paused = true;
    }

    void
    Start()
    {
        paused = false;
        last_frame = SDL_GetTicks();
    }
};


struct position
{
    position() {}
    //position(const Position &x) : row(x.row), col(x.col) {}
    position(int col, int row) : col(col), row(row) {}

    int col, row;

    position& operator+=(const position& rhs)
    {
        this->col += rhs.col;
        this->row += rhs.row;
        return *this;
    }
};
bool
operator<(const position &a, const position &b)
{
    return a.row < b.row || (!(b.row < a.row) && a.col < b.col);
}
bool
operator==(const position &a, const position &b)
{
    return a.col == b.col && a.row == b.row;
}
position
operator+(const position &a, const position &b)
{
    return {a.col + b.col, a.row + b.row};
}
position
operator-(const position &a, const position &b)
{
    return {a.col - b.col, a.row - b.row};
}
position
operator*(const position &a, int i)
{
    return {a.col * i, a.row * i};
}
position
operator*(const position &a, float f)
{
    return {(int)(a.col * f), (int)(a.row * f)};
}
std::ostream
&operator<<(std::ostream &os, position const &p)
{
    return os << "(" << p.col << ", " << p.row << ")";
}

typedef position direction;
typedef vector<position> path;



// REST ////////////////////////////
// Returns a value clamped between min and max.
int
clamp(int val, int min, int max)
{
    if(val < min)
        return min;
    else if(val > max)
        return max;
    return val;
}

position
clamp(const position &pos, const position &min, const position &max)
{
    position result = pos;
    if(pos.col <= min.col)
        result.col = min.col;
    else if(pos.col > max.col)
        result.col = max.col;

    if(pos.row <= min.row)
        result.row = min.row;
    else if(pos.row > max.row)
        result.row = max.row;

    return result;
}

// ===================================== Viewport ===============================
// Checks if a point is within the current viewport.
bool
WithinViewport(const position &p)
{
    return (p.col < VIEWPORT_WIDTH + viewportCol &&
            p.col >= viewportCol &&
            p.row < VIEWPORT_HEIGHT + viewportRow &&
            p.row >= viewportRow);
}

// moves the cursor's viewport once so that the given tile is on screen.
// Very Impure
void
MoveViewport(const position &p)
{
    if(WithinViewport(p))
        return;

    if(p.col >= VIEWPORT_WIDTH + viewportCol)
    {
        ++viewportCol;
    }
    else if(p.col < viewportCol)
    {
        --viewportCol;
    }
    else if(p.row >= VIEWPORT_HEIGHT + viewportRow)
    {
        ++viewportRow;
    }
    else if(p.row < viewportRow)
    {
        --viewportRow;
    }
}

// Sets the viewport to have the point p inside of it.
// Meant to be called at the start of the level.
void
SetViewport(const position &p, int width, int height)
{
    if(WithinViewport(p))
        return;

    position viewport_grid_position = 
    {
        p.col / VIEWPORT_WIDTH,
        p.row / VIEWPORT_HEIGHT
    };
    viewportCol = clamp(viewport_grid_position.col * VIEWPORT_WIDTH,
                        0, width - VIEWPORT_WIDTH);
    viewportRow = clamp(viewport_grid_position.row * VIEWPORT_HEIGHT,
                        0, height - VIEWPORT_HEIGHT);
}

// ================================= Library ===================================
SDL_Color
LerpColors(const SDL_Color &one, const SDL_Color &two, float ratio)
{
    return {
            (Uint8)(one.r * (1.0 - ratio) + (two.r * ratio)),
            (Uint8)(one.g * (1.0 - ratio) + (two.g * ratio)),
            (Uint8)(one.b * (1.0 - ratio) + (two.b * ratio)),
            (Uint8)(one.a * (1.0 - ratio) + (two.a * ratio))
            };
}

SDL_Color
PiecewiseColors(const SDL_Color &one, const SDL_Color &two, float ratio)
{
    if(ratio > 0.5)
        return one;
    return two;
}
SDL_Color
PiecewiseColors(const SDL_Color &one, const SDL_Color &two, const SDL_Color &three, float ratio)
{
    if(ratio < 0.33)
        return one;
    else if(ratio < 0.66)
        return two;
    return three;
}

ImU32
SdlToImColor(const SDL_Color &in)
{
    return IM_COL32(in.r, in.g, in.b, in.a);
}

/* TODO
SDL_Color
ImToSdlColor(ImU32 in)
{
    return {in.r, in.g, in.b, in.a};
}
*/

#endif
