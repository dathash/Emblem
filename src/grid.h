// Author: Alex Hartford
// Program: Emblem
// File: Grid Functions
// Date: July 2022

#ifndef GRID_H
#define GRID_H

// ========================= grid helper functions ========================
// returns true if the position is in-bounds.
bool
IsValidBoundsPosition(int col, int row)
{
    return (col >= 0 && col < MAP_SIZE
         && row >= 0 && row < MAP_SIZE);
}


// returns true if a given pair is in a vector of pairs.
bool
VectorHasElement(const pair<int, int> &pair_in, const vector<pair<int, int>> &vector_in)
{
    bool has = false;
    for(pair<int, int> p : vector_in)
    {
        if(pair_in == p)
        {
            has = true;
        }
    }
    return has;
}


// returns a vector of pairs representing accessible squares for a given unit.
vector<pair<int, int>>
AccessibleFrom(const Tilemap &map, int col, int row, int max, bool sourceIsAlly)
{
    vector<pair<int, int>> accessible;
    int costs[MAP_SIZE][MAP_SIZE];

    // initialize all elements to high value.
    for(int i = 0; i < MAP_SIZE; ++i)
        for(int j = 0; j < MAP_SIZE; ++j)
            costs[i][j] = 100;

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<pair<int, int>> unexplored;
    unexplored.push(make_pair(col, row));
    costs[col][row] = 0;
    
    while(!unexplored.empty())
    { 
        pair<int, int> current = unexplored.front();
        unexplored.pop();

        accessible.push_back(current);

        // Add adjacent tiles to the list!
        for(int i = 0; i < 4; ++i)
        {
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(newCol, newRow))
            {
                const Tile *tileToExplore = &map.tiles[newCol][newRow];
                int newCost;
                if(map.tiles[newCol][newRow].occupied && map.tiles[newCol][newRow].occupant->isAlly != sourceIsAlly)
                {
                    newCost = 100;
                }
                else
                {
                    newCost = costs[current.first][current.second] + map.tiles[newCol][newRow].penalty;
                }
                if(newCost < costs[newCol][newRow])
                {
                    costs[newCol][newRow] = newCost;
                    if(costs[newCol][newRow] <= max)
                    {
                        unexplored.push(make_pair(newCol, newRow));
                    }
                }
            }
        }
    }

    accessible.erase(remove_if(accessible.begin(), accessible.end(),
            [map, col, row](pair<int, int> p)
            {
                return (!(p.first == col && p.second == row) &&
                        map.tiles[p.first][p.second].occupied);
            }),
            accessible.end());
    return accessible;
}

// returns a vector of pairs representing accessible squares for a given unit.
vector<pair<int, int>>
InteractibleFrom(const Tilemap &map, int col, int row, int min, int max)
{
    vector<pair<int, int>> interactible;
    int costs[MAP_SIZE][MAP_SIZE];

    // initialize all elements to high value.
    for(int i = 0; i < MAP_SIZE; ++i)
        for(int j = 0; j < MAP_SIZE; ++j)
            costs[i][j] = 100;

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<pair<int, int>> unexplored;
    unexplored.push(make_pair(col, row));
    costs[col][row] = 0;
    
    while(!unexplored.empty())
    { 
        pair<int, int> current = unexplored.front();
        unexplored.pop();

        interactible.push_back(current);

        // Add adjacent tiles to the list!
        for(int i = 0; i < 4; ++i)
        {
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(newCol, newRow))
            {
                const Tile *tileToExplore = &map.tiles[newCol][newRow];
                int newCost = costs[current.first][current.second] + 1;
                if(newCost < costs[newCol][newRow])
                {
                    costs[newCol][newRow] = newCost;
                    if(costs[newCol][newRow] <= max)
                    {
                        unexplored.push(make_pair(newCol, newRow));
                    }
                }
            }
        }
    }

    interactible.erase(remove_if(interactible.begin(), interactible.end(),
            [&costs, min](pair<int, int> p) { return costs[p.first][p.second] < min; }),
            interactible.end());
    return interactible;
}


// Finds the nearest unit to the cursor, based on the given predicate expression.
shared_ptr<Unit> FindNearest(const Cursor &cursor, const Tilemap &map, bool predicate(const Unit &))
{
    int minDistance = 100;
    int distance = 0;
    shared_ptr<Unit> result = nullptr;
    for(int col = 0; col < MAP_SIZE; ++col)
    {
        for(int row = 0; row < MAP_SIZE; ++row)
        {
            if(map.tiles[col][row].occupied && predicate(*map.tiles[col][row].occupant))
            {
                distance = abs(col - cursor.col) + abs(row - cursor.row);
                if(distance < minDistance)
                {
                    result = map.tiles[col][row].occupant;
                    minDistance = distance;
                }
            }
        }
    }
    return result;
}


// Finds the nearest accessible space to a given target point.
// TODO: Do something better than manhattan distance.
pair<int, int> FindClosestAccessibleTile(const Tilemap &map, int col, int row)
{
    int minDistance = 100;
    int distance = 0;
    pair<int, int> result;

    for(pair<int, int> p : map.accessible)
    {
        // manhattan distance
        distance = abs(p.first - col) + abs(p.second - row);
        if(distance < minDistance)
        {
            result = p;
            minDistance = distance;
        }
    }

    return result;
}


// Finds a target for an attack.
shared_ptr<Unit> FindVictim(const Cursor &cursor, const Tilemap &map)
{
    shared_ptr<Unit> result = nullptr;
    for(pair<int, int> p : map.interactible)
    {
        printf("%d %d\n", p.first, p.second);
        if(map.tiles[p.first][p.second].occupied && map.tiles[p.first][p.second].occupant->isAlly)
        {
            result = map.tiles[p.first][p.second].occupant;
        }
    }
    return result;
}


#endif
