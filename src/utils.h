
// Author: Alex Hartford
// Program: Emblem
// File: Utils
// Date: January 2023

#ifndef UTILS_H
#define UTILS_H

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
