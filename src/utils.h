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
bool operator<(const position &a, const position &b)
{
    return a.row < b.row || (!(b.row < a.row) && a.col < b.col);
}
bool operator==(const position &a, const position &b)
{
    return a.col == b.col && a.row == b.row;
}
position operator+(const position &a, const position &b)
{
    return {a.col + b.col, a.row + b.row};
}
position operator-(const position &a, const position &b)
{
    return {a.col - b.col, a.row - b.row};
}
std::ostream &operator<<(std::ostream &os, position const &p) { 
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

#endif
