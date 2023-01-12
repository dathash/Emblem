// Author: Alex Hartford
// Program: Emblem
// File: Grid

#ifndef GRID_H
#define GRID_H

#include <queue>

// ===================================== Viewport ===============================
// Checks if a point is within the current viewport.
bool
WithinViewport(int col, int row)
{
    return (col < VIEWPORT_WIDTH + viewportCol &&
            col >= viewportCol &&
            row < VIEWPORT_HEIGHT + viewportRow &&
            row >= viewportRow);
}

// moves the cursor's viewport so that the given tile is on screen.
// NOTE: creates side effect. Unpredictable.
void
MoveViewport(int col, int row)
{
    if(WithinViewport(col, row))
    {
        return;
    }
    if(col >= VIEWPORT_WIDTH + viewportCol)
    {
        ++viewportCol;
    }
    else if(col < viewportCol)
    {
        --viewportCol;
    }
    else if(row >= VIEWPORT_HEIGHT + viewportRow)
    {
        ++viewportRow;
    }
    else if(row < viewportRow)
    {
        --viewportRow;
    }
}


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

// Finds the manhattan distance between two points.
int ManhattanDistance(const point &one, const point &two)
{
    return (abs(one.first - two.first) + abs(one.second - two.second));
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


void
PrintField(const vector<vector<point>> &field)
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


// Get a field of directions which indicate shortest paths to a specified node.
// Also produces a Distance Field, which indicates distance at each point.
// NOTE: Currently just prints out the distance field.
vector<vector<point>>
GetField(const Tilemap &map, int col, int row, bool isAlly)
{
    vector<vector<point>> field;
	for(int col = 0; col < map.width; ++col)
	{
        vector<point> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(point(-1, -1));
        }
        field.push_back(currentColumn);
	}

    vector<vector<int>> distances;
	for(int col = 0; col < map.width; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(100);
        }
        distances.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<point> unexplored;
    unexplored.push(make_pair(col, row));
    field[col][row] = point(-2, -2);
    if(map.tiles[col][row].type == WALL)
    {
        field[col][row] = point(-1, -1);
    }
    distances[col][row] = 0;

    while(!unexplored.empty())
    {
        point current = unexplored.front();
        unexplored.pop();

        for(int i = 0; i < 4; ++i)
        {
            // NOTE: This is strange. I shouldn't have to swap them.
            point direction = point(-directionsRow[i], -directionsCol[i]);
            int newCol = current.first + directionsCol[i];
            int newRow = current.second + directionsRow[i];
            if(IsValidBoundsPosition(map.width, map.height, newCol, newRow))
            {
                int newCost = distances[current.first][current.second]
                              + map.tiles[newCol][newRow].penalty;

                if(map.tiles[newCol][newRow].occupant &&
                   (map.tiles[newCol][newRow].occupant->isAlly != isAlly))
                {
                    newCost = 100;
                }
                
                else if(newCost < distances[newCol][newRow])
                {
                    field[newCol][newRow] = direction;
                    distances[newCol][newRow] = newCost;
                    unexplored.push(make_pair(newCol, newRow));
                }
            }
        }
    }
#if 0
    cout << "=============================\n";
    PrintDistanceField(distances);
    PrintField(field);
#endif

    return field;
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

// TODO: Col and row get swapped for some reason. I put in a hotfix.
path
GetPath(const Tilemap &map,
        int col, int row,
        int destCol, int destRow,
        bool isAlly)
{
    vector<point> path;
    vector<vector<point>> field = GetField(map, destCol, destRow, isAlly);

    point next = point(col, row);
    point from = field[next.first][next.second];
    while(!(from.first == -2 && from.second == -2) &&
          !(from.first == -1 && from.second == -1))
    {
        path.push_back(next);
        from = field[next.first][next.second];
        next = point(next.first + from.second,
                     next.second + from.first);
    }
    return path;
}


// Returns the furthest point down a path that a unit could move in a round.
// A necessary workaround due to the fact that units can move through allies
// but cannot land on their squares.
point
FurthestMovementOnPath(const Tilemap &map, const path &full_path, int movement)
{
    for(int i = movement; i > 0; --i) // Start at the furthest square, test all.
    {
        if(!map.tiles[full_path[i].first][full_path[i].second].occupant)
        {
            return full_path[i];
        }
    }
    assert(!"This will be a problem.\n");
    return {0, 0};
}


//  ================================ NEW AI METHODS ============================
void
PrintPossibilities(const vector<pair<point, Unit *>> &v)
{
    cout << "Possilibities:\n";
    for(const pair<point, Unit *> &p : v)
    {
        cout << "Point: " << p.first.first << " " << p.first.second << " | ";
        cout << p.second->name << "\n";
    }
}

// Finds all possible squares for attacking enemies.
// SLOW: This shouldn't have to do exhaustive search. How about going through
// the enemy units instead?
vector<pair<point, Unit *>>
FindAttackingSquares(const Tilemap &map, const Unit &unit)
{
    vector<pair<point, Unit *>> result = {};
    vector<point> interactible;

    for(const point &p : map.accessible)
    {
        interactible = InteractibleFrom(map, p.first, p.second, unit.minRange, unit.maxRange);
        for(const point &i : interactible)
        {
            if(map.tiles[i.first][i.second].occupant && map.tiles[i.first][i.second].occupant->isAlly)
            {
                result.push_back(pair<point, Unit *>(p, map.tiles[i.first][i.second].occupant));
            }
        }
    }

    return result;
}

//TODO: This is still doing things naively.
// Finds the nearest unit to the cursor, based on the given predicate expression.
Unit *FindNearest(const point &p, const Tilemap &map, bool predicate(const Unit &))
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
                distance = ManhattanDistance(p, point(col, row));
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


#endif
