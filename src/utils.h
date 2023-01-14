// Author: Alex Hartford
// Program: Emblem
// File: Utils

#ifndef UTILS_H
#define UTILS_H


struct position
{
    position() {}
    //position(const Position &x) : row(x.row), col(x.col) {}
    position(int col, int row) : col(col), row(row) {}

    int col, row;
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
std::ostream
&operator<<(std::ostream &os, position const &p)
{
    return os << "(" << p.col << ", " << p.row << ")";
}

typedef position direction;
typedef vector<position> path;


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

// moves the cursor's viewport so that the given tile is on screen.
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

#endif
