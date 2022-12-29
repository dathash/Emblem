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

// returns true if a given point is in a vector.
bool
VectorHasElement(const point &point_in, const vector<point> &vector_in)
{
    bool has = false;
    for(point p : vector_in)
    {
        if(point_in == p)
        {
            has = true;
        }
    }
    return has;
}


// returns a vector of points representing accessible squares for a given unit.
vector<point>
AccessibleFrom(const Tilemap &map, int col, int row, int max, bool sourceIsAlly)
{
    vector<point> accessible;

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

    queue<point> unexplored;
    unexplored.push(make_pair(col, row));
    costs[col][row] = 0;
    
    while(!unexplored.empty())
    { 
        point current = unexplored.front();
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
                if(map.tiles[newCol][newRow].occupant && map.tiles[newCol][newRow].occupant->isAlly != sourceIsAlly)
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
            [map, col, row](point p)
            {
                return (!(p.first == col && p.second == row) &&
                        map.tiles[p.first][p.second].occupant);
            }),
            accessible.end());
    return accessible;
}

// returns a vector of pairs representing accessible squares for a given unit.
vector<point>
InteractibleFrom(const Tilemap &map, int col, int row, int min, int max)
{
    vector<point> interactible;

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

    queue<point> unexplored;
    unexplored.push(make_pair(col, row));
    costs[col][row] = 0;
    
    while(!unexplored.empty())
    { 
        point current = unexplored.front();
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
            [&costs, min](point p) { return costs[p.first][p.second] < min; }),
            interactible.end());
    return interactible;
}

// Super simple distance equation.
float ManhattanDistance(int col, int row, int tar_col, int tar_row)
{
    return (abs(tar_col - col) + abs(tar_row - row));
}

//TODO: This is still doing things naively.
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
            if(map.tiles[col][row].occupant && predicate(*map.tiles[col][row].occupant))
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
point FindClosestAccessibleTile(const Tilemap &map, int col, int row)
{
    int minDistance = 100;
    int distance = 0;
    point result;

    for(point p : map.accessible)
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
    for(point p : map.interactible)
    {
        if(map.tiles[p.first][p.second].occupant && map.tiles[p.first][p.second].occupant->isAlly)
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
    point result;
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

    queue<point> unexplored;
    unexplored.push(make_pair(col, row));
    field[col][row] = 0;

    while(!unexplored.empty())
    { 
        point current = unexplored.front();
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
PrintFromsField(const vector<vector<point>> &field)
{
    for(vector<point> row : field)
    {
        for(point val : row)
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
vector<vector<point>>
FromsField(const Tilemap &map, int col, int row)
{
    //TODO: look up what this field should be called haha
    vector<vector<point>> froms;
	for(int col = 0; col < map.width; ++col)
	{
        vector<point> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(point(-1, -1));
        }
        froms.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<point> unexplored;
    unexplored.push(make_pair(col, row));
    froms[col][row] = point(-2, -2);

    while(!unexplored.empty())
    { 
        point current = unexplored.front();
        unexplored.pop();
        for(int i = 0; i < 4; ++i)
        {
            point direction = point( -directionsRow[i], -directionsCol[i]);
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
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
PrintPath(const vector<point> path)
{
    cout << "PATH:\n";
    for(point point : path)
    {
        cout << point.first << " " << point.second << "\n";
    }
}

// TODO: Col and row get swapped for some reason. I fixed it, but it's suck.
vector<point>
GetPath(const Tilemap &map,
        int col, int row,
        int destCol, int destRow)
{
    vector<point> path;
    vector<vector<point>> froms = FromsField(map, destCol, destRow);

    point next = point(col, row);
	point from = point(0, 0);
    while(!(from.first == -2 && from.second == -2))
    {
        path.push_back(next);
        from = froms[next.first][next.second];
        next = point(next.first + from.second,
                     next.second + from.first);
        if(from.first == -1 && from.second == -1)
        {
            cout << "ERROR: Targeting Wall.\n";
            return {};
        }
    }
    return path;
}

#endif
