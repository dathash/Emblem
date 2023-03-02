// Author: Alex Hartford
// Program: Emblem
// File: Equipment

#ifndef EQUIP_H
#define EQUIP_H

enum EffectType
{
    EFFECT_NONE,
    EFFECT_AFLAME,
    EFFECT_PARALYZED,
    EFFECT_SWIFT,
    EFFECT_STONE,
};
struct Effect
{
    EffectType type = EFFECT_NONE;
};
string GetEffectString(EffectType type) {
    switch(type)
    {
        case EFFECT_NONE:       return "No Effect";
        case EFFECT_AFLAME:     return "Aflame";
        case EFFECT_PARALYZED:  return "Para";
        case EFFECT_SWIFT:      return "Swift";
        case EFFECT_STONE:      return "Stone";
    }
}
Effect
GetEffectByName(const string &name)
{
    if(name == " ") return {};
    else if(name == "Aflame") return {EFFECT_AFLAME};
    else if(name == "Para")   return {EFFECT_PARALYZED};
    else if(name == "Swift")  return {EFFECT_SWIFT};
    else if(name == "Stone")  return {EFFECT_STONE};
    else cout << "WARN GetEffectByName: Incorrect name " << name << "\n"; return {};
}


enum ClassType
{
    CLASS_NONE,
    CLASS_EARTH,
    CLASS_WIND,
    CLASS_FIRE,
    CLASS_STORM,
    CLASS_UTILITY,
    CLASS_AI,
};
string
GetClassString(ClassType type)
{
    switch(type)
    {
        case CLASS_NONE:     return "None";
        case CLASS_EARTH:    return "Earth";
        case CLASS_WIND:     return "Wind";
        case CLASS_FIRE:     return "Fire";
        case CLASS_STORM:    return "Storm";
        case CLASS_UTILITY:  return "Utility";
        case CLASS_AI:       return "AI";
        default:             return "";
    }
}

enum EquipmentType
{
    EQUIP_NONE,
    EQUIP_PUNCH,
    EQUIP_LINE_SHOT,
    EQUIP_ARTILLERY,
    EQUIP_SELF_TARGET,
    EQUIP_LEAP,
    EQUIP_LASER,
};
string
GetEquipmentString(EquipmentType type)
{
    switch(type)
    {
        case EQUIP_NONE:        return "None";
        case EQUIP_PUNCH:       return "Punch";
        case EQUIP_LINE_SHOT:   return "Line";
        case EQUIP_ARTILLERY:   return "Artillery";
        case EQUIP_SELF_TARGET: return "Self";
        case EQUIP_LEAP:        return "Leap";
        case EQUIP_LASER:       return "Laser";
    }
}

enum PushType
{
    PUSH_NONE,
    PUSH_AWAY,
    PUSH_TOWARDS,
    PUSH_TOWARDS_AND_AWAY,
    PUSH_PERPENDICULAR,
    PUSH_ALL,
};
string
GetPushString(PushType type)
{
    switch(type)
    {
        case PUSH_NONE:     return "None";
        case PUSH_AWAY:     return "Away";
        case PUSH_TOWARDS:  return "Towards";
        case PUSH_TOWARDS_AND_AWAY:  return "T+A";
        case PUSH_PERPENDICULAR:     return "Perp";
        case PUSH_ALL:      return "All";
        default:                return "";
    }
}

enum MovementType
{
    MOVEMENT_NONE,
    MOVEMENT_BACKONE,
    MOVEMENT_RAM,
    MOVEMENT_LEAP,
};
string
GetMovementString(MovementType type)
{
    switch(type)
    {
        case MOVEMENT_NONE:     return "None";
        case MOVEMENT_BACKONE:  return "Back one";
        case MOVEMENT_RAM:      return "Ram";
        case MOVEMENT_LEAP:     return "Leap";
    }
}

struct Equip
{
    string name;
    EquipmentType type        = EQUIP_NONE;
    ClassType     cls         = CLASS_NONE;
    PushType      push        = PUSH_NONE;
    MovementType  move        = MOVEMENT_NONE;
    EffectType    effect      = EFFECT_NONE;

    int damage      = 1;
    int push_damage = 1;
    int self_damage = 0;

    int min_range = 1;
    int max_range = 1;

    Equip(
         string name_in,
         EquipmentType type_in,
         ClassType cls_in,
         PushType push_in,
         MovementType move_in,
         EffectType effect_in,
         int damage_in,
         int push_damage_in,
         int self_damage_in,
         int min_range_in,
         int max_range_in)
    : name(name_in),
      type(type_in),
      cls(cls_in),
      push(push_in),
      move(move_in),
      effect(effect_in),
      damage(damage_in),
      push_damage(push_damage_in),
      self_damage(self_damage_in),
      min_range(min_range_in),
      max_range(max_range_in)
    {}

    size_t ID() { return hash<string>{}(name); }
};

Equip *
GetEquipByName(const vector<shared_ptr<Equip>> &equipments, const string &name)
{
    if(name == " ") return nullptr;

    for(shared_ptr<Equip> equip : equipments) {
        if(equip->ID() == hash<string>{}(name))
            return equip.get();
    }
    cout << "WARN GetEquipByName: No equip of that name: " << name << "\n";
    return nullptr;
}

#endif
