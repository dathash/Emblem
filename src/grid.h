// Author: Alex Hartford
// Program: Emblem
// File: Grid

#ifndef GRID_H
#define GRID_H

#include <queue>

// ========================= grid helper functions ========================
// returns true if the position is in-bounds.
bool
IsValidBoundsPosition(int mapWidth, int mapHeight, const position &pos)
{
    return (pos.col >= 0 && pos.col < mapWidth &&
			pos.row >= 0 && pos.row < mapHeight);
}

// returns true if a given point is in a vector.
bool
VectorHasElement(const position &pos_in, const vector<position> &vector_in)
{
    bool has = false;
    for(const position &p : vector_in)
    {
        if(pos_in == p)
        {
            has = true;
        }
    }
    return has;
}


// returns a vector of positions representing accessible squares for a given unit.
vector<position>
InteractibleFrom(const Tilemap &map, const position &origin, int min, int max)
{
    vector<position> interactible;

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
            if(IsValidBoundsPosition(map.width, map.height, new_pos))
            {
                int newCost = costs[current.col][current.row] + 1;
                if(newCost < costs[new_pos.col][new_pos.row])
                {
                    costs[new_pos.col][new_pos.row] = newCost;
                    if(costs[new_pos.col][new_pos.row] <= max)
                    {
                        unexplored.push(new_pos);
                    }
                }
            }
        }
    }

    interactible.erase(remove_if(interactible.begin(), interactible.end(),
            [&costs, min](const position &p) { return costs[p.col][p.row] < min; }),
            interactible.end());
    return interactible;
}

// returns a vector of positions representing accessible squares for a given unit.
// NOTE: This may be the most hideous function I have ever written.
// TODO: Immolate this
// TODO: This produces doubles. For accessible and attackable. Fix!
pair<vector<position>, vector<position>>
AccessibleAndAttackableFrom(const Tilemap &map, position origin, 
                            int mov, int min, int max, 
                            bool sourceIsAlly)
{
    vector<position> accessible;
    vector<position> attackable;

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
            if(IsValidBoundsPosition(map.width, map.height, new_pos))
            {
                int newCost;
                if(map.tiles[new_pos.col][new_pos.row].occupant && 
                   map.tiles[new_pos.col][new_pos.row].occupant->is_ally != sourceIsAlly)
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
                    {
                        unexplored.push(new_pos);
                    }
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

    for(const position &p : accessible)
    {
        vector<position> from_here = InteractibleFrom(map, p, min, max);
        for(const position &res : from_here)
        {
            attackable.push_back(res);
        }
    }

    // SO SLOOOOOW
    attackable.erase(remove_if(attackable.begin(), attackable.end(),
            [map, accessible](const position &p)
            {
                for(const position &mask : accessible)
                {
                    if(p == mask)
                        return true;
                }
                return false;
            }),
            attackable.end());

    return pair<vector<position>, vector<position>>(accessible, attackable);
}

// Finds the manhattan distance between two positions.
int ManhattanDistance(const position &one, const position &two)
{
    return (abs(one.col - two.col) + abs(one.row - two.row));
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
// NOTE: Currently just prints out the distance field.
vector<vector<direction>>
GetField(const Tilemap &map, position origin, bool is_ally)
{
    vector<vector<direction>> field;
	for(int col = 0; col < map.width; ++col)
	{
        vector<direction> currentColumn = {};
        for(int row = 0; row < map.height; ++row)
        {
            currentColumn.push_back(direction(-1, -1));
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

    queue<position> unexplored;
    unexplored.push(origin);
    field[origin.col][origin.row] = direction(-2, -2);
    if(map.tiles[origin.col][origin.row].type == WALL)
    {
        field[origin.col][origin.row] = direction(-1, -1);
    }
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
            if(IsValidBoundsPosition(map.width, map.height, new_pos))
            {
                int newCost = distances[current.col][current.row]
                              + map.tiles[new_pos.col][new_pos.row].penalty;

                if(map.tiles[new_pos.col][new_pos.row].occupant &&
                   (map.tiles[new_pos.col][new_pos.row].occupant->is_ally != is_ally))
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
#if 0
    cout << "=============================\n";
    PrintDistanceField(distances);
    PrintField(field);
#endif

    return field;
}

void
PrintPath(const path &path_in)
{
    cout << "PATH:\n";
    for(const position &pos : path_in)
    {
        cout << pos << "\n";
    }
}

// Given a start and end position, returns the shortest path between them,
// taking into account a given "is_ally" value to determine impassible unit tiles.
path
GetPath(const Tilemap &map,
        position start,
        position destination,
        bool is_ally)
{
    path path_result;
    vector<vector<position>> field = GetField(map, destination, is_ally);

    position next = start;
    direction from = field[next.col][next.row];
    while(!(from.col == -2 && from.row == -2) &&
          !(from.col == -1 && from.row == -1))
    {
        path_result.push_back(next);
        from = field[next.col][next.row];
        // TODO: Col and row get swapped for some reason. I put in a hotfix.
        next = position(next.col + from.row,
                        next.row + from.col);
    }
    return path_result;
}


// Returns the furthest point down a path that a unit could move in a round.
// A necessary workaround due to the fact that units can move through allies
// but cannot land on their squares.
position
FurthestMovementOnPath(const Tilemap &map, const path &path_in, int movement)
{
    SDL_assert(path_in.size());
    if(movement > path_in.size())
    {
        return {0, 0};
    }
    for(int i = movement; i > 0; --i) // Start at the furthest square, test all.
    {
        if(!map.tiles[path_in[i].col][path_in[i].row].occupant)
        {
            return path_in[i];
        }
    }
    return {0, 0};
}


//  ================================ NEW AI METHODS ============================
void
PrintPossibilities(const vector<pair<position, Unit *>> &v)
{
    cout << "Possilibities:\n";
    for(const pair<position, Unit *> &poss : v)
    {
        cout << "Point: " << poss.first.col << " " << poss.first.row << " | ";
        cout << poss.second->name << "\n";
    }
}

// Finds all possible squares for attacking enemies.
// SLOW: This shouldn't have to do exhaustive search. How about going through
// the enemy units instead?
vector<pair<position, Unit *>>
FindAttackingSquares(const Tilemap &map, const Unit &unit,
                     const vector<position> &range)
{
    vector<pair<position, Unit *>> result = {};
    vector<position> interactible;

    for(const position &pos : range)
    {
        interactible = InteractibleFrom(map, pos, unit.min_range, unit.max_range);
        for(const position &i : interactible)
        {
            if(map.tiles[i.col][i.row].occupant && map.tiles[i.col][i.row].occupant->is_ally)
            {
                result.push_back(pair<position, Unit *>(pos, map.tiles[i.col][i.row].occupant));
            }
        }
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
    for(int col = 0; col < map.width; ++col)
    {
        for(int row = 0; row < map.height; ++row)
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
