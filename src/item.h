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
    int might = -1;
    int min_range = -1;
    int max_range = -1;
    int weight = -1;
    int hit = -1;
    //int durability = -1;
    //int max_durability = -1;

    WeaponComponent(WeaponType type_in, int might_in, 
                    int min_range_in, int max_range_in, 
                    int weight_in, int hit_in)
    : type(type_in),
      might(might_in),
      min_range(min_range_in),
      max_range(max_range_in),
      weight(weight_in),
      hit(hit_in)
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
    int uses = -1;
    
    ConsumableComponent(ConsumableType type_in, int amount_in, int uses_in)
    : type(type_in),
      amount(amount_in),
      uses(uses_in)
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
    ITEM_NONE,
    ITEM_SWORD_IRON,
    ITEM_SWORD_STEEL,
    ITEM_SWORD_SILVER,
    ITEM_SPEAR_IRON,
    ITEM_SPEAR_STEEL,
    ITEM_SPEAR_SILVER,
    ITEM_SPEAR_JAVELIN,
    ITEM_AXE_IRON,
    ITEM_AXE_STEEL,
    ITEM_AXE_SILVER,
    ITEM_AXE_HANDAXE,
    ITEM_BOW_IRON,
    ITEM_BOW_STEEL,
    ITEM_BOW_SILVER,
    ITEM_BOW_LONGBOW,
    ITEM_TOME_SPARK,
    ITEM_TOME_BOLT,
    ITEM_TOME_STORM,
    ITEM_STAFF_HEAL,
    ITEM_STAFF_MEND,
    ITEM_STAFF_REVIVE,

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
    case ITEM_NONE:         return "NONE";
    case ITEM_SWORD_IRON:   return "Iron Sword";
    case ITEM_SWORD_STEEL:  return "Steel Sword";
    case ITEM_SWORD_SILVER: return "Silver Sword";
    case ITEM_SPEAR_IRON:   return "Iron Spear";
    case ITEM_SPEAR_STEEL:  return "Steel Spear";
    case ITEM_SPEAR_SILVER: return "Silver Spear";
    case ITEM_SPEAR_JAVELIN:return "Javelin";
    case ITEM_AXE_IRON  :   return "Iron Axe";
    case ITEM_AXE_STEEL :   return "Steel Axe";
    case ITEM_AXE_SILVER:   return "Silver Axe";
    case ITEM_AXE_HANDAXE:  return "Handaxe";
    case ITEM_BOW_IRON  :   return "Iron Bow";
    case ITEM_BOW_STEEL :   return "Steel Bow";
    case ITEM_BOW_SILVER:   return "Silver Bow";
    case ITEM_BOW_LONGBOW:  return "Longbow";
    case ITEM_TOME_SPARK:   return "Spark";
    case ITEM_TOME_BOLT :   return "Bolt";
    case ITEM_TOME_STORM:   return "Storm";
    case ITEM_STAFF_HEAL:   return "Heal";
    case ITEM_STAFF_MEND:   return "Mend";
    case ITEM_STAFF_REVIVE: return "Revive";
    case ITEM_POTION_LOW:   return "Potion I";
    case ITEM_POTION_MID:   return "Potion II";
    case ITEM_POTION_HIGH:  return "Potion III";
    case ITEM_SHIELD    :   return "Shield";
    case ITEM_HELMET    :   return "Helmet";
    case ITEM_WIND_RING :   return "Wind Ring";
    default: cout << "WARNING GetItemString: Unhandled " << type << "\n"; return "";
    }
}

struct Item
{
    ItemType type;
    WeaponComponent *weapon = nullptr;
    ConsumableComponent *consumable = nullptr;
    EquipmentComponent *equipment = nullptr;

    Item(ItemType type_in,
         WeaponComponent *weapon_in = nullptr,
         ConsumableComponent *consumable_in = nullptr,
         EquipmentComponent *equipment_in = nullptr)
    : type(type_in),
      weapon(weapon_in),
      consumable(consumable_in),
      equipment(equipment_in)
    {
    }

    ~Item()
    {
        delete weapon;
        delete consumable;
        delete equipment;
    }

    Item(const Item &other)
    {
        type = other.type;
        if(other.weapon)
            weapon = new WeaponComponent(*other.weapon);
        if(other.consumable)
            consumable = new ConsumableComponent(*other.consumable);
        if(other.equipment)
            equipment = new EquipmentComponent(*other.equipment);
    }
};

Item *
GetItem(ItemType type)
{
    switch(type)
    {
        case ITEM_NONE:           return nullptr;
        // Weapons
        case ITEM_SWORD_IRON:     return new Item(type, new WeaponComponent(WEAPON_SWORD,   5, 1, 1, 3, 90));
        case ITEM_SWORD_STEEL:    return new Item(type, new WeaponComponent(WEAPON_SWORD,   8, 1, 1, 5, 80));
        case ITEM_SWORD_SILVER:   return new Item(type, new WeaponComponent(WEAPON_SWORD,   10, 1, 1, 5, 80));

        case ITEM_SPEAR_IRON:     return new Item(type, new WeaponComponent(WEAPON_SPEAR,   7, 1, 1, 6, 80));
        case ITEM_SPEAR_STEEL:    return new Item(type, new WeaponComponent(WEAPON_SPEAR,   10, 1, 1, 8, 70));
        case ITEM_SPEAR_SILVER:   return new Item(type, new WeaponComponent(WEAPON_SPEAR,   14, 1, 1, 9, 70));
        case ITEM_SPEAR_JAVELIN:  return new Item(type, new WeaponComponent(WEAPON_SPEAR,   6, 1, 2, 10, 60));

        case ITEM_AXE_IRON:       return new Item(type, new WeaponComponent(WEAPON_AXE,     6, 1, 1, 5, 70));
        case ITEM_AXE_STEEL:      return new Item(type, new WeaponComponent(WEAPON_AXE,     9, 1, 1, 7, 60));
        case ITEM_AXE_SILVER:     return new Item(type, new WeaponComponent(WEAPON_AXE,     13, 1, 1, 8, 60));
        case ITEM_AXE_HANDAXE:    return new Item(type, new WeaponComponent(WEAPON_AXE,     5, 1, 2, 8, 50));

        case ITEM_BOW_IRON:       return new Item(type, new WeaponComponent(WEAPON_BOW,     5, 2, 2, 4, 110));
        case ITEM_BOW_STEEL:      return new Item(type, new WeaponComponent(WEAPON_BOW,     7, 2, 2, 5, 100));
        case ITEM_BOW_SILVER:     return new Item(type, new WeaponComponent(WEAPON_BOW,     9, 2, 2, 6, 100));
        case ITEM_BOW_LONGBOW:    return new Item(type, new WeaponComponent(WEAPON_BOW,     5, 2, 3, 7, 60));

        case ITEM_TOME_SPARK:     return new Item(type, new WeaponComponent(WEAPON_TOME,    3, 1, 1, 1, 90));
        case ITEM_TOME_BOLT:      return new Item(type, new WeaponComponent(WEAPON_TOME,    9, 1, 1, 2, 80));
        case ITEM_TOME_STORM:     return new Item(type, new WeaponComponent(WEAPON_TOME,    16, 1, 1, 3, 60));

        case ITEM_STAFF_HEAL:     return new Item(type, new WeaponComponent(WEAPON_STAFF,   10, 1, 1, 1, 100));
        case ITEM_STAFF_MEND:     return new Item(type, new WeaponComponent(WEAPON_STAFF,   15, 1, 1, 1, 100));
        case ITEM_STAFF_REVIVE:   return new Item(type, new WeaponComponent(WEAPON_STAFF,   30, 1, 3, 1, 100));

        // Consumables
        case ITEM_POTION_LOW:     return new Item(type, nullptr, new ConsumableComponent(CONS_POTION, 10, 3));
        case ITEM_POTION_MID:     return new Item(type, nullptr, new ConsumableComponent(CONS_POTION, 20, 3));
        case ITEM_POTION_HIGH:    return new Item(type, nullptr, new ConsumableComponent(CONS_POTION, 30, 3));

        // Equipment
        case ITEM_SHIELD:         return new Item(type, nullptr, nullptr, new EquipmentComponent(EQUIP_ARMOR, 2));
        case ITEM_HELMET:         return new Item(type, nullptr, nullptr, new EquipmentComponent(EQUIP_ARMOR, 1));

        case ITEM_WIND_RING:      return new Item(type, nullptr, nullptr, new EquipmentComponent(EQUIP_RING,  3));

        default: cout << "WARNING ItemIndex: No item at index " << type << "\n"; return nullptr;
    }
}

#endif
