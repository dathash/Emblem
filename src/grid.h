// Author: Alex Hartford
// Program: Emblem
// File: Grid

#ifndef GRID_H
#define GRID_H

#include <queue>

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
VectorHasElement(const position &pos_in, const vector<position> &vector_in)
{
    for(const position &p : vector_in)
        if(pos_in == p)
           return true;

    return false;
}

// returns a vector of positions representing accessible squares for a given unit.
vector<position>
Interactible(const Tilemap &map, const position &origin, int range)
{
    vector<position> interactible;

	// initialize costs matrix
	vector<vector<int>> costs;
	for(int col = 0; col < MAP_WIDTH; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < MAP_HEIGHT; ++row)
            currentColumn.push_back(100);

        costs.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<position> unexplored;
    unexplored.push(origin);
    costs[origin.col][origin.row] = 0;
    
    while(!unexplored.empty())
    { 
        position current = unexplored.front();
        unexplored.pop();

        interactible.push_back(current);

        // Add adjacent tiles to the list!
        for(int i = 0; i < 4; ++i)
        {
            position new_pos = {current.col + directionsCol[i],
                                current.row + directionsRow[i]};
            if(IsValid(new_pos))
            {
                int newCost = costs[current.col][current.row] + 1;
                if(newCost < costs[new_pos.col][new_pos.row])
                {
                    costs[new_pos.col][new_pos.row] = newCost;
                    if(costs[new_pos.col][new_pos.row] <= range)
                        unexplored.push(new_pos);
                }
            }
        }
    }

    interactible.erase(remove_if(interactible.begin(), interactible.end(),
            [&costs, range](const position &p) { return costs[p.col][p.row] < range; }),
            interactible.end());
    return interactible;
}

// TODO: Other weapon types
// returns a vector of positions representing orthogonal squares for a given unit.
vector<position>
Orthogonal(const Tilemap &map, const position &origin)
{
    vector<position> orthogonal = {};

    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        if(col != origin.col)
            orthogonal.push_back(position(col, origin.row));
    }
    for(int row = 0; row < MAP_HEIGHT; ++row)
    {
        if(row != origin.row)
            orthogonal.push_back(position(origin.col, row));
    }

    assert(orthogonal.size() == (MAP_WIDTH - 1) + (MAP_HEIGHT - 1));
    return orthogonal;
}

// returns a vector of positions representing squares in a line from a point.
vector<position>
Line(const Tilemap &map, const position &origin, const direction &dir)
{
    vector<position> line = {};

    position current = origin + dir;
    while(IsValid(current))
    {
        line.push_back(current);
        current = current + dir;
    }

    return line;
}

// returns a vector of positions representing accessible squares for a given unit.
vector<position>
Accessible(const Tilemap &map, position origin, 
           int mov, bool sourceIsAlly)
{
    vector<position> accessible;

	// initialize costs matrix
	vector<vector<int>> costs;
	for(int col = 0; col < MAP_WIDTH; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            currentColumn.push_back(100);
        }
        costs.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<position> unexplored;
    unexplored.push(origin);
    costs[origin.col][origin.row] = 0;
    
    while(!unexplored.empty())
    { 
        position current = unexplored.front();
        unexplored.pop();

        accessible.push_back(current);

        // Add adjacent tiles to the list!
        for(int i = 0; i < 4; ++i)
        {
            position new_pos = {current.col + directionsCol[i],
                                current.row + directionsRow[i]};
            if(IsValid(new_pos))
            {
                int newCost;
                if(map.tiles[new_pos.col][new_pos.row].occupant && 
                   map.tiles[new_pos.col][new_pos.row].occupant->IsAlly() != sourceIsAlly)
                {
                    newCost = 100;
                }
                else
                {
                    newCost = costs[current.col][current.row] +
                              map.tiles[new_pos.col][new_pos.row].penalty;
                }
                if(newCost < costs[new_pos.col][new_pos.row])
                {
                    costs[new_pos.col][new_pos.row] = newCost;
                    if(costs[new_pos.col][new_pos.row] <= mov)
                        unexplored.push(new_pos);
                }
            }
        }
    }

    accessible.erase(remove_if(accessible.begin(), accessible.end(),
            [map, origin](const position &p)
            {
                return (!(p == origin) &&
                        map.tiles[p.col][p.row].occupant);
            }),
            accessible.end());

    return accessible;
}

// Assumes orthogonality
// Determines if a straight path between two points is unobstructed.
bool
Unobstructed(const Tilemap &map, const position &one, const position &two)
{
    direction dir = GetDirection(one, two);
    position cur = one + dir;
    while(cur != two)
    {
        if(map.tiles[cur.col][cur.row].occupant)
            return false;
        cur = cur + dir;
    }
    return true;
}

void
PrintField(const vector<vector<position>> &field)
{
    for(vector<position> row : field)
    {
        for(position p : row)
        {
            string dir = "o";
            if(p.col == -1 && p.row == -1)
            {
                dir = "o";
            }
            else if(p.col == -2 && p.row == -2)
            {
                dir = "x";
            }
            else if(p.col == -1 && p.row == 0)
            {
                dir = "<";
            }
            else if(p.col == 1 && p.row == 0)
            {
                dir = ">";
            }
            else if(p.col == 0 && p.row == -1)
            {
                dir = "^";
            }
            else if(p.col == 0 && p.row == 1)
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
vector<vector<direction>>
GetField(const Tilemap &map, position origin, bool is_ally)
{
    vector<vector<direction>> field;
	for(int col = 0; col < MAP_WIDTH; ++col)
	{
        vector<direction> currentColumn = {};
        for(int row = 0; row < MAP_HEIGHT; ++row)
            currentColumn.push_back(direction(-1, -1));
        field.push_back(currentColumn);
	}

    vector<vector<int>> distances;
	for(int col = 0; col < MAP_WIDTH; ++col)
	{
        vector<int> currentColumn = {};
        for(int row = 0; row < MAP_HEIGHT; ++row)
            currentColumn.push_back(100);
        distances.push_back(currentColumn);
	}

    int directionsRow[] = { -1,  0,  1,  0 };
    int directionsCol[] = {  0,  1,  0, -1 };

    queue<position> unexplored;
    unexplored.push(origin);
    field[origin.col][origin.row] = direction(-2, -2);
    if(map.tiles[origin.col][origin.row].type == WALL)
        field[origin.col][origin.row] = direction(-1, -1);

    distances[origin.col][origin.row] = 0;

    while(!unexplored.empty())
    {
        position current = unexplored.front();
        unexplored.pop();

        for(int i = 0; i < 4; ++i)
        {
            // NOTE: This is strange. I shouldn't have to swap them.
            direction dir = {-directionsRow[i], -directionsCol[i]};
            position new_pos = {current.col + directionsCol[i],
                                current.row + directionsRow[i]};
            if(IsValid(new_pos))
            {
                int newCost = distances[current.col][current.row] + 
                              map.tiles[new_pos.col][new_pos.row].penalty;

                if(map.tiles[new_pos.col][new_pos.row].occupant &&
                   (map.tiles[new_pos.col][new_pos.row].occupant->IsAlly() != is_ally))
                {
                    newCost = 100;
                }
                
                else if(newCost < distances[new_pos.col][new_pos.row])
                {
                    field[new_pos.col][new_pos.row] = dir;
                    distances[new_pos.col][new_pos.row] = newCost;
                    unexplored.push(new_pos);
                }
            }
        }
    }

    PrintField(field);

    return field;
}

// Given a start and end position, returns the shortest path between them,
// taking into account a given "is_ally" value to determine impassible unit tiles.
path
GetPath(const Tilemap &map,
        position start,
        position destination,
        bool is_ally)
{
    path result;
    vector<vector<position>> field = GetField(map, destination, is_ally);

    position next = start;
    direction from = field[next.col][next.row];
    while(!(from.col == -2 && from.row == -2) &&
          !(from.col == -1 && from.row == -1))
    {
        result.push_back(next);
        from = field[next.col][next.row];
        // TODO: Col and row get swapped for some reason. I put in a hotfix.
        next = position(next.col + from.row,
                        next.row + from.col);
    }
    return result;
}


// Returns the furthest point down a path that a unit could move in a round.
// A necessary workaround due to the fact that units can move through allies
// but cannot land on their squares.
position
FurthestMovementOnPath(const Tilemap &map, const path &path_in, int movement)
{
    SDL_assert(!path_in.empty());
    if(movement >= path_in.size())
        return {0, 0};

    cout << "=====================================\n";
    cout << path_in.size() <<"\n";
    for(int i = movement; i > 0; --i) // Start at the furthest square, test all.
    {
        cout << i << "\n";
        if(!map.tiles[path_in[i].col][path_in[i].row].occupant)
            return path_in[i];
    }

    return {0, 0};
}


//  ================================ NEW AI METHODS ============================

// Finds all possible squares for attacking enemies.
vector<pair<position, Unit *>>
FindAttackingSquares(const Tilemap &map, const Unit &unit,
                     const vector<position> &range)
{
    vector<pair<position, Unit *>> result = {};
    vector<position> interactible;

    for(const position &pos : range)
    {
        interactible = Interactible(map, pos, unit.range);
        for(const position &i : interactible)
            if(map.tiles[i.col][i.row].occupant && map.tiles[i.col][i.row].occupant->IsAlly())
                result.push_back(pair<position, Unit *>(pos, map.tiles[i.col][i.row].occupant));
    }

    return result;
}

// Finds the nearest unit to the cursor, based on the given predicate expression.
Unit *FindNearest(const Tilemap &map, const position &origin, 
                  bool predicate(const Unit &), bool is_ally)
{
    int minDistance = 100;
    int distance = 0;
    Unit *result = nullptr;
    for(int col = 0; col < MAP_WIDTH; ++col)
    {
        for(int row = 0; row < MAP_HEIGHT; ++row)
        {
            if(map.tiles[col][row].occupant && predicate(*map.tiles[col][row].occupant))
            {
                distance = GetPath(map, origin, position(col, row), is_ally).size();
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
