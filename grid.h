// Author: Alex Hartford
// Program: Emblem
// File: Grid Functions
// Date: July 2022

/*
   TODO
   InteractibleFrom
 */

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
shared_ptr<vector<pair<int, int>>>
AccessibleFrom(const Tilemap &map, int col, int row, int max)
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
                int newCost = costs[current.first][current.second] + map.tiles[newCol][newRow].penalty;
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
    return make_shared<vector<pair<int, int>>>(accessible);
}

// returns a vector of pairs representing accessible squares for a given unit.
shared_ptr<vector<pair<int, int>>>
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
    return make_shared<vector<pair<int, int>>>(interactible);
}

#endif
