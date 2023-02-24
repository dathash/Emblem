// Author: Alex Hartford
// Program: Emblem
// File: Test

#ifndef TEST_H
#define TEST_H

void
RunTests()
{
    // utils.h
    assert(position(4, 6) == position(4, 6));
    assert(position(4, 6) != position(5, 6));
    assert(position(1, 2) + position(3, 2) == position(4, 4));
    assert(position(1, 2) - position(0, 1) == position(1, 1));
    assert(position(1, 2) * 2 == position(2, 4));

    assert(Length(direction(-10, 0)) == 10);
    assert(Length(direction(0, 4)) == 4);
    assert(Length(direction(40, 0)) == 40);

    assert(Normalized(direction(4, 0)) == direction(1, 0));
    assert(Normalized(direction(0, 0)) == direction(0, 0));
    assert(Normalized(direction(50, 0)) == direction(1, 0));
    assert(Normalized(direction(-10, 0)) == direction(-1, 0));

    assert(GetDirection({0, 0}, {1, 0}) == direction(1, 0));
    assert(GetDirection({1, 0}, {0, 0}) == direction(-1, 0));
    assert(GetDirection({0, 0}, {0, 0}) == direction(0, 0));
    assert(GetDirection({0, 1}, {0, 4}) == direction(0, 1));
}

#endif
