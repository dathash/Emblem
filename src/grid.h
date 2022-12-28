// Author: Alex Hartford
// Program: Emblem
// File: Grid Functions
// Date: July 2022

#ifndef GRID_H
#define GRID_H

// ========================= grid helper functions ========================
// returns true if the position is in-bounds.
bool
IsValidBoundsPosition(int mapWidth, int mapHeight, int col, int row)
{
    return (col >= 0 && col < mapWidth &&
			row >= 0 && row < mapHeight);
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

	// initialize costs matrix
	vector<vector<int>> costs;
	for(int col = 0; col < map.width; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(100);
        }
        costs.push_back(currentColumn);
	}

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
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
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

	// initialize costs matrix
	vector<vector<int>> costs;
	for(int col = 0; col < map.width; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(100);
        }
        costs.push_back(currentColumn);
	}

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
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
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

// Super simple distance equation.
float ManhattanDistance(int col, int row, int tar_col, int tar_row)
{
    return (abs(tar_col - col) + abs(tar_row - row));
}

// A* Pathfinding distance equation.

// Finds the nearest unit to the cursor, based on the given predicate expression.
Unit *FindNearest(const Cursor &cursor, const Tilemap &map, bool predicate(const Unit &))
{
    int minDistance = 100;
    int distance = 0;
    Unit *result = nullptr;
    for(int col = 0; col < map.width; ++col)
    {
        for(int row = 0; row < map.height; ++row)
        {
            if(map.tiles[col][row].occupied && predicate(*map.tiles[col][row].occupant))
            {
                distance = ManhattanDistance(cursor.col, cursor.row, col, row);
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
        distance = ManhattanDistance(col, row, p.first, p.second);
        if(distance < minDistance)
        {
            result = p;
            minDistance = distance;
        }
    }

    return result;
}

// Finds a target for an attack.
Unit *FindVictim(const Cursor &cursor, const Tilemap &map)
{
    Unit *result = nullptr;
    for(pair<int, int> p : map.interactible)
    {
        if(map.tiles[p.first][p.second].occupied && map.tiles[p.first][p.second].occupant->isAlly)
        {
            result = map.tiles[p.first][p.second].occupant;
        }
        // determine nearest enemy here
        // (add possible enemies to a list, sort.)
    }
    return result;
}

void
PrintDistanceField(const vector<vector<int>> &field)
{
    for(vector<int> row : field)
    {
        for(int val : row)
        {
            cout << std::setw(3) << val << " ";
        }
        cout << "\n";
    }
}

vector<vector<int>>
DistanceField(const Tilemap &map, int col, int row)
{
    vector<vector<int>> field;
    int minDistance = 100;
    int distance = 0;
    pair<int, int> result;
	for(int col = 0; col < map.width; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(100);
        }
        field.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<pair<int, int>> unexplored;
    unexplored.push(make_pair(col, row));
    field[col][row] = 0;

    while(!unexplored.empty())
    { 
        pair<int, int> current = unexplored.front();
        unexplored.pop();

        for(int i = 0; i < 4; ++i)
        {
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
                int newCost = field[current.first][current.second] + map.tiles[newCol][newRow].penalty;
                if(newCost < field[newCol][newRow])
                {
                    field[newCol][newRow] = newCost;
                    unexplored.push(make_pair(newCol, newRow));
                }
            }
        }
    }

    return field;
}

void
PrintFromsField(const vector<vector<pair<int, int>>> &field)
{
    for(vector<pair<int, int>> row : field)
    {
        for(pair<int, int> val : row)
        {
            string dir = "o";
            if(val.first == -1 && val.second == -1)
            {
                dir = "o";
            }
            else if(val.first == -2 && val.second == -2)
            {
                dir = "x";
            }
            else if(val.first == -1 && val.second == 0)
            {
                dir = "<";
            }
            else if(val.first == 1 && val.second == 0)
            {
                dir = ">";
            }
            else if(val.first == 0 && val.second == -1)
            {
                dir = "^";
            }
            else if(val.first == 0 && val.second == 1)
            {
                dir = "v";
            }
            cout << dir << " ";
        }
        cout << "\n";
    }
}


// Get a field of pairs which indicate where they came from.
vector<vector<pair<int, int>>>
FromsField(const Tilemap &map, int col, int row)
{
    //TODO: look up what this field should be called haha
    vector<vector<pair<int, int>>> froms;
	for(int col = 0; col < map.width; ++col)
	{
        vector<pair<int, int>> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(pair<int, int>(-1, -1));
        }
        froms.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<pair<int, int>> unexplored;
    unexplored.push(make_pair(col, row));
    froms[col][row] = pair<int, int>(-2, -2);

    while(!unexplored.empty())
    { 
        pair<int, int> current = unexplored.front();
        unexplored.pop();
        cout << current.first << " " << current.second << "\n";
        cout << "New ones:\n";
        for(int i = 0; i < 4; ++i)
        {
            pair<int, int> direction = pair<int, int>( -directionsRow[i], -directionsCol[i]);
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
                cout << newCol << " " << newRow << "\n";
                cout << "Pen: " << map.tiles[newCol][newRow].penalty << "\n";
                if(froms[newCol][newRow].first == -1 &&
                   froms[newCol][newRow].second == -1 &&
                   map.tiles[newCol][newRow].penalty == 1)
                    // is froms algorithm always shortest path?
                    // TODO: doesn't handle penalties (forest, desert, etc.)
                {
                    froms[newCol][newRow] = direction;
                    unexplored.push(make_pair(newCol, newRow));
                }
            }
        }
    }

    return froms;
}

void
PrintPath(const vector<pair<int, int>> path)
{
    cout << "PATH:\n";
    for(pair<int, int> point : path)
    {
        cout << point.first << " " << point.second << "\n";
    }
}

// TODO: Col and row get swapped for some reason. I fixed it, but it's suck.
vector<pair<int, int>>
GetPath(const Tilemap &map,
        int col, int row,
        int destCol, int destRow)
{
    vector<pair<int, int>> path;
    vector<vector<pair<int, int>>> froms = FromsField(map, destCol, destRow);
    PrintFromsField(froms);
    pair<int, int> next = pair<int, int>(col, row);
	pair<int, int> from = pair<int, int>(0, 0);
    while(!(from.first == -2 && from.second == -2))
    {
        path.push_back(next);
		from = froms[next.first][next.second];
        next = pair<int, int>(next.first + from.second,
                              next.second + from.first);
        if(from.first == -1 && from.second == -1)
        {
            cout << "ERROR: Targeting Wall.\n";
            return path;
        }
    }
    PrintPath(path);
    return path;
}

#endif
