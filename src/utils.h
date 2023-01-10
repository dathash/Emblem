// Author: Alex Hartford
// Program: Emblem
// File: Utils

#ifndef UTILS_H
#define UTILS_H

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
