// Author: Alex Hartford
// Program: Emblem
// File: Weapon

#ifndef WEAPON_H
#define WEAPON_H

enum ItemType
{
    ITEM_NONE,
    ITEM_SWORD,
    ITEM_BOW,
    ITEM_SPARK,
    ITEM_EMBER,
    ITEM_GUST,
    ITEM_STAFF_HEAL,
    ITEM_POTION_LOW,
    ITEM_POTION_MID,
    ITEM_POTION_HIGH,
    ITEM_SHIELD,
    ITEM_HELMET,
    ITEM_WIND_RING,
};

string
GetItemString(ItemType type)
{
    switch(type)
    {
    case ITEM_NONE:       return "NONE";
    case ITEM_SWORD:      return "Sword";
    case ITEM_BOW:        return "Bow";
    case ITEM_SPARK:      return "Spark";
    case ITEM_EMBER:      return "Ember";
    case ITEM_GUST:       return "Gust";
    case ITEM_STAFF_HEAL: return "Staff of Healing";
    case ITEM_POTION_LOW: return "Potion I";
    case ITEM_POTION_MID: return "Potion II";
    case ITEM_POTION_HIGH:return "Potion III";
    case ITEM_SHIELD:     return "Shield";
    case ITEM_HELMET:     return "Helmet";
    case ITEM_WIND_RING:  return "Ring of Wind";
    default: cout << "WARNING GetItemString: Unhandled " << type << "\n"; return "";
    }
}

enum ClassType
{
    CLASS_NONE,
    CLASS_PRIME,
    CLASS_BRUTE,
    CLASS_RANGED,
    CLASS_SCIENCE,
    CLASS_SUPPORT,
};

enum DamageType
{
    DAMAGE_NONE,
    DAMAGE_ADJACENT,
    DAMAGE_CLOSEST,
    DAMAGE_PATH,
    DAMAGE_LINE,

};

enum KnockbackType
{
    KNOCKBACK_NONE,
    KNOCKBACK_AWAY,
    KNOCKBACK_TOWARDS,
    KNOCKBACK_PARALLEL,
    KNOCKBACK_PERPENDICULAR,
    KNOCKBACK_ALL,
};

enum MovementType
{
    MOVEMENT_NONE,
    MOVEMENT_RAM,
    MOVEMENT_BACKONE,
    MOVEMENT_LEAP,
};

enum EffectType
{
    EFFECT_NONE,
    EFFECT_ACID,
    EFFECT_FIRE,
    EFFECT_SMOKE,
    EFFECT_SHIELD,
    EFFECT_ROCK,
};

struct Item
{
    ItemType type = ITEM_NONE;
    ClassType weapon_class = CLASS_NONE;
    KnockbackType knockback = KNOCKBACK_NONE;
    int damage = 1;
    int self_damage = 0;
};

Item
GetItem(ItemType type)
{
    switch(type)
    {
    case ITEM_NONE:       return {};
    case ITEM_SWORD:      return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_BOW:        return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_SPARK:      return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_EMBER:      return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_GUST:       return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_STAFF_HEAL: return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_POTION_LOW: return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_POTION_MID: return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_POTION_HIGH:return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_SHIELD:     return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_HELMET:     return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    case ITEM_WIND_RING:  return {type, CLASS_PRIME, KNOCKBACK_AWAY, 2, 0};
    default: cout << "WARNING ItemIndex: No item at index " << type << "\n"; return {};
    }
}

#endif
