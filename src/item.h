// Author: Alex Hartford
// Program: Emblem
// File: Item

#ifndef ITEM_H
#define ITEM_H

// Weapons
enum WeaponType
{
    WEAPON_NOTHING,
    WEAPON_SPEAR,
    WEAPON_SWORD,
    WEAPON_AXE,
    WEAPON_BOW,
    WEAPON_TOME,
    WEAPON_STAFF,
};

struct WeaponComponent
{
    WeaponType type = WEAPON_NOTHING;
    int damage = -1;
    int min_range = -1;
    int max_range = -1;
    int weight = -1;
    //int durability = -1;
    //int max_durability = -1;

    WeaponComponent(WeaponType type_in, int damage_in, 
                    int min_range_in, int max_range_in, 
                    int weight_in)
    : type(type_in),
      damage(damage_in),
      min_range(min_range_in),
      max_range(max_range_in),
      weight(weight_in)
    {}
};

// Consumables
enum ConsumableType
{
    CONS_NOTHING,
    CONS_POTION,
};

struct ConsumableComponent
{
    ConsumableType type = CONS_NOTHING;
    int amount = -1;
    
    ConsumableComponent(ConsumableType type_in, int amount_in)
    : type(type_in),
      amount(amount_in)
    {}
};

// Equipment
enum EquipmentType
{
    EQUIP_NOTHING,
    EQUIP_ARMOR,
    EQUIP_RING,
};

struct EquipmentComponent
{
    EquipmentType type = EQUIP_NOTHING;
    int amount = -1;

    EquipmentComponent(EquipmentType type_in, int amount_in)
    : type(type_in),
      amount(amount_in)
    {}
};

enum ItemType
{
    ITEM_NOTHING,
    ITEM_WEAPON,
    ITEM_CONSUMABLE,
    ITEM_EQUIPMENT,
};

struct Item
{
    //ItemType type = ITEM_NOTHING;
    WeaponComponent *weapon = nullptr;
    ConsumableComponent *consumable = nullptr;
    EquipmentComponent *equipment = nullptr;

    ~Item()
    {
        delete weapon;
        delete consumable;
        delete equipment;
    }
};

enum ItemValue
{
    IV_SWORD_IRON,
    IV_SWORD_STEEL,
    IV_SWORD_SILVER,
    IV_SPEAR_IRON,
    IV_SPEAR_STEEL,
    IV_SPEAR_SILVER,
    IV_SPEAR_JAVELIN,
    IV_AXE_IRON,
    IV_AXE_STEEL,
    IV_AXE_SILVER,
    IV_AXE_HANDAXE,
    IV_BOW_IRON,
    IV_BOW_STEEL,
    IV_BOW_SILVER,
    IV_BOW_LONGBOW,
    IV_TOME_SPARK,
    IV_TOME_BOLT,
    IV_TOME_STORM,
    IV_STAFF_HEAL,
    IV_STAFF_MEND,
    IV_STAFF_REVIVE,

    IV_POTION_LOW,
    IV_POTION_MID,
    IV_POTION_HIGH,

    IV_SHIELD,
    IV_HELMET,
    IV_WIND_RING,
};

Item
ItemIndex(int index)
{
    switch(index)
    {
        // Weapons
        case IV_SWORD_IRON:     return {new WeaponComponent(WEAPON_SWORD,   5, 1, 1, 3)};
        case IV_SWORD_STEEL:    return {new WeaponComponent(WEAPON_SWORD,   8, 1, 1, 5)};
        case IV_SWORD_SILVER:   return {new WeaponComponent(WEAPON_SWORD,   10, 1, 1, 5)};

        case IV_SPEAR_IRON:     return {new WeaponComponent(WEAPON_SPEAR,   7, 1, 1, 6)};
        case IV_SPEAR_STEEL:    return {new WeaponComponent(WEAPON_SPEAR,   10, 1, 1, 8)};
        case IV_SPEAR_SILVER:   return {new WeaponComponent(WEAPON_SPEAR,   14, 1, 1, 9)};
        case IV_SPEAR_JAVELIN:  return {new WeaponComponent(WEAPON_SPEAR,   6, 1, 2, 10)};

        case IV_AXE_IRON:       return {new WeaponComponent(WEAPON_AXE,     6, 1, 1, 5)};
        case IV_AXE_STEEL:      return {new WeaponComponent(WEAPON_AXE,     9, 1, 1, 7)};
        case IV_AXE_SILVER:     return {new WeaponComponent(WEAPON_AXE,     13, 1, 1, 8)};
        case IV_AXE_HANDAXE:    return {new WeaponComponent(WEAPON_AXE,     5, 1, 2, 8)};

        case IV_BOW_IRON:       return {new WeaponComponent(WEAPON_BOW,     5, 2, 2, 4)};
        case IV_BOW_STEEL:      return {new WeaponComponent(WEAPON_BOW,     7, 2, 2, 5)};
        case IV_BOW_SILVER:     return {new WeaponComponent(WEAPON_BOW,     9, 2, 2, 6)};
        case IV_BOW_LONGBOW:    return {new WeaponComponent(WEAPON_BOW,     5, 2, 3, 7)};

        case IV_TOME_SPARK:     return {new WeaponComponent(WEAPON_TOME,    3, 1, 1, 1)};
        case IV_TOME_BOLT:      return {new WeaponComponent(WEAPON_TOME,    9, 1, 1, 2)};
        case IV_TOME_STORM:     return {new WeaponComponent(WEAPON_TOME,    16, 1, 1, 3)};

        case IV_STAFF_HEAL:     return {new WeaponComponent(WEAPON_STAFF,   10, 1, 1, 1)};
        case IV_STAFF_MEND:     return {new WeaponComponent(WEAPON_STAFF,   15, 1, 1, 1)};
        case IV_STAFF_REVIVE:   return {new WeaponComponent(WEAPON_STAFF,   30, 1, 3, 1)};

        // Consumables
        case IV_POTION_LOW:     return {nullptr, new ConsumableComponent(CONS_POTION, 10)};
        case IV_POTION_MID:     return {nullptr, new ConsumableComponent(CONS_POTION, 20)};
        case IV_POTION_HIGH:    return {nullptr, new ConsumableComponent(CONS_POTION, 30)};

        // Equipment
        case IV_SHIELD:         return {nullptr, nullptr, new EquipmentComponent(EQUIP_ARMOR, 2)};
        case IV_HELMET:         return {nullptr, nullptr, new EquipmentComponent(EQUIP_ARMOR, 1)};

        case IV_WIND_RING:      return {nullptr, nullptr, new EquipmentComponent(EQUIP_RING,  3)};

        default: cout << "WARNING ItemIndex: No item at index " << index << "\n"; return {};
    }
}

#endif
