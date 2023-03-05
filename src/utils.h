// Author: Alex Hartford
// Program: Emblem
// File: Utils

#ifndef UTILS_H
#define UTILS_H

// ============================= Dice ==========================================
// Rolls a d100. range: 00 to 99.
int d100() {
    return rand() % 100;
}
int d1() {
    return 1;
}
// range: 01 to 04. (etc.)
int d4() {
    return (rand() % 4) + 1;
}
int d6() {
    return (rand() % 6) + 1;
}
int d8() {
    return (rand() % 8) + 1;
}
int d10() {
    return (rand() % 10) + 1;
}
int d12() {
    return (rand() % 12) + 1;
}
int d20() {
    return (rand() % 20) + 1;
}

// Rolls a number of dice.
int Roll(int (*die)(), int num = 1) {
    int result = 0;
    for(int i = 0; i < num; ++i)
        result += die();
    return result;
}

string
GetDieString(int (*die)())
{
    if(die == d1)
        return "d1";
    else if(die == d4)
        return "d4";
    else if(die == d6)
        return "d6";
    else if(die == d8)
        return "d8";
    else if(die == d10)
        return "d10";
    else if(die == d12)
        return "d12";
    cout << "WARNING: Bad die format: " << die << "\n";
    return "";
}

int
GetMaxValue(int (*die)())
{
    if(die == d1)
        return 1;
    else if(die == d4)
        return 4;
    else if(die == d6)
        return 6;
    else if(die == d8)
        return 8;
    else if(die == d10)
        return 10;
    else if(die == d12)
        return 12;
    cout << "WARNING: Bad die format: " << die << "\n";
    return 0;
}


// Range: (0, max)
int RandomInt(int max) {
    return rand() % (max + 1);
}


// ================================= Structs ===================================
struct Timer
{
    Uint32 current = 0;
    Uint32 last_frame = 0;
    Uint32 end;
    bool paused = false;

    Timer(int seconds) {
        last_frame = SDL_GetTicks();
        end = seconds * 1000;
    }

    bool Update() {
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

    void Pause() {
        paused = true;
    }

    void Start() {
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
operator==(const position &a, const position &b)
{
    return a.col == b.col && a.row == b.row;
}
bool
operator!=(const position &a, const position &b)
{
    return a.col != b.col || a.row != b.row;
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
std::ostream
&operator<<(std::ostream &os, position const &p)
{
    return os << "(" << p.col << ", " << p.row << ")";
}

typedef position direction;
int Length(const direction &dir_in) {
    return abs(dir_in.col) + abs(dir_in.row);
}

direction Normalized(const direction &dir_in) {
    int length = Length(dir_in);
    if(!length) return {0, 0};
    return {dir_in.col / length, dir_in.row / length};
}

// Finds the direction from one position to another.
direction GetDirection(const position &one, const position &two) {
    return Normalized(two - one);
}
// Finds the distance from one position to another.
int Distance(const position &one, const position &two) {
    return Length(two - one);
}

// ========================= grid helper functions ========================
// returns true if the position is in-bounds.
bool
IsValid(const position &pos)
{
    return (pos.col >= 0 && pos.col < MAP_WIDTH &&
			pos.row >= 0 && pos.row < MAP_HEIGHT);
}

// returns true if a given point is in a vector.
bool
VectorContains(const position &pos, const vector<position> &vector_in)
{
    for(const position &p : vector_in)
        if(pos == p)
            return true;
    return false;
}

bool IsEdge(const position &pos)
{
    return (pos.col == 0 || pos.col == MAP_WIDTH - 1
         || pos.row == 0 || pos.row == MAP_HEIGHT - 1);
}

bool Warpable(const position &pos)
{
    return pos.row > 3 && !IsEdge(pos);
}


typedef vector<position> path;

int
min(int a, int b)
{
    if(a < b)
        return a;
    return b;
}

int
max(int a, int b)
{
    if(a > b)
        return a;
    return b;
}

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

ImU32 SdlToImColor(const SDL_Color &in) {
    return IM_COL32(in.r, in.g, in.b, in.a);
}

// ======================== Easing Functions ===================================
float Lerp(float a, float b, float amount) {
    return a * (1.0 - amount) + (b * amount);
}

float Identity(float t) {
    return t;
}

float Flip(float t) {
    return 1 - t;
}

float SmoothStart2(float t) {
    return t * t;
}

float SmoothStart3(float t) {
    return t * t * t;
}

float SmoothStart4(float t) {
    return t * t * t * t;
}

float SmoothStart5(float t) {
    return t * t * t * t * t;
}

float SmoothStop2(float t) {
    return Flip(SmoothStart2(Flip(t)));
}

float SmoothStop3(float t) {
    return Flip(SmoothStart3(Flip(t)));
}

float SmoothStop4(float t) {
    return Flip(SmoothStart4(Flip(t)));
}

float SmoothStop5(float t) {
    return Flip(SmoothStart5(Flip(t)));
}

float SmoothStartStop(float t) {
    return Lerp(SmoothStart2(t), SmoothStop2(t), t);
}

float SmoothStartStopCustom(float t, float (*in) (float), float (*out) (float)) {
    return Lerp(in(t), out(t), t);
}

float Spike(float t) {
    if (t <= .5f)
        return SmoothStart2(t/0.5);
 
    return SmoothStart2(Flip(t)/0.5);
}

float Parabola(float t) {
    return t * (1 - t);
}

float BounceClampBottom(float t) {
    return fabs(t);
}

float BounceClampTop(float t) {
    return 1.0f - fabs(1.0f - t);
}

float BounceClampBottomTop(float t) {
    return BounceClampTop(BounceClampTop(t));
}

#endif
