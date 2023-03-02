// Author: Alex Hartford
// Program: Emblem
// File: Resolution

#ifndef RESOLUTION_H
#define RESOLUTION_H

enum IncidentType
{
    INCIDENT_ATTACK,
    INCIDENT_AFLAME,
    INCIDENT_ENVIRONMENT,
};
string GetIncidentString(IncidentType type)
{
    switch(type)
    {
        case INCIDENT_ATTACK:   return "Attack";
        case INCIDENT_AFLAME:   return "Aflame";
        case INCIDENT_ENVIRONMENT: return "Environment";
    }
}


// CIRCULAR
void
Simulate(Tilemap *map,
         const Equip &weapon, 
         position source, 
         position destination);

struct Incident
{
    Unit *unit;
    position offset;
    IncidentType type = INCIDENT_ATTACK;

    void Resolve(Tilemap *map) {
        switch(type)
        {
            case INCIDENT_ATTACK:
            {
                if(!unit->HasEffect(EFFECT_PARALYZED))
                    Simulate(map, *unit->primary, unit->pos, unit->pos + offset);
            } break;
            case INCIDENT_AFLAME:
            {
                unit->Damage(1);
                cout << "Fire damage!!!\n";
            } break;
            case INCIDENT_ENVIRONMENT:
            {
            } break;
        }
    }
};

struct Resolution
{
    vector<Incident> incidents = {};
    int frame = 0;

    bool Update(Tilemap *map) {
        if(incidents.empty())
            return true;

        ++frame;
        if(frame % AI_ACTION_SPEED)
            return false;

        incidents.back().Resolve(map);
        incidents.pop_back();
        return false;
    }

    void Clear() {
        incidents.clear();
    }

    void RemoveDeadUnits() {
        incidents.erase(remove_if(incidents.begin(), incidents.end(),
                    [](auto const &a) { return a.unit->should_die || a.unit->HasEffect(EFFECT_PARALYZED); }),
                    incidents.end());
    }
};

#endif
