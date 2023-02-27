// Author: Alex Hartford
// Program: Emblem
// File: Resolution

#ifndef RESOLUTION_H
#define RESOLUTION_H

struct Attack
{
    Unit *unit;
    position offset;

    void Resolve(Tilemap *map) {
        Simulate(map, *unit->primary, unit->pos, unit->pos + offset);
    }
};

struct Resolution
{
    vector<Attack> attacks;
    int frame = 0;

    void Update(Tilemap *map) {
        if(attacks.empty())
            GoToAIPhase();

        ++frame;
        if(frame % AI_ACTION_SPEED)
            return;

        attacks.back().Resolve(map);
        attacks.pop_back();
    }

    void Clear() {
        attacks.clear();
    }

    void RemoveDeadUnits() {
        attacks.erase(remove_if(attacks.begin(), attacks.end(),
                    [](auto const &a) { return a.unit->should_die; }),
                    attacks.end());
    }
};

#endif
