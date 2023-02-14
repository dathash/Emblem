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

enum Stat
{
    STAT_NONE,
    STAT_STRENGTH,
    STAT_DEXTERITY,
    STAT_VITALITY,
    STAT_INTUITION,
    STAT_FAITH,
};

struct WeaponComponent
{
    int (*die)() = d6;
    int num_dice = 1;
    Stat hit_stat = STAT_STRENGTH;
    Stat dmg_stat = STAT_STRENGTH;
    int min_range = 1;
    int max_range = 1;
    int bonus_to_hit = 0;
    int bonus_damage = 0;

    WeaponComponent(int (*die_in)(), int num_dice_in = 1,
                    Stat hit_stat_in = STAT_STRENGTH, Stat dmg_stat_in = STAT_STRENGTH,
                    int min_range_in = 1, int max_range_in = 1,
                    int bonus_to_hit_in = 0, int bonus_damage_in = 0
                    )
    : die(die_in),
      num_dice(num_dice_in),
      hit_stat(hit_stat_in),
      dmg_stat(dmg_stat_in),
      min_range(min_range_in),
      max_range(max_range_in),
      bonus_to_hit(bonus_to_hit_in),
      bonus_damage(bonus_damage_in)
    {
    }

    int
    RollDamage()
    {
        int result = bonus_damage;
        for(int i = 0; i < num_dice; ++i)
        {
            result += die();
        }
        return result;
    }
};

// Consumables
enum ConsumableType
{
    CONS_NOTHING,
    CONS_POTION,
    CONS_STATBOOST,
    CONS_BUFF,
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
    ITEM_CLUB,
    ITEM_DAGGER,
    ITEM_SLING,
    ITEM_BROADSWORD,
    ITEM_SHORTSWORD,
    ITEM_GREATSWORD,
    ITEM_SPEAR,
    ITEM_AXE,
    ITEM_HANDAXE,
    ITEM_BOW,
    ITEM_LONGBOW,
    ITEM_SPARK,
    ITEM_EMBER,
    ITEM_GUST,
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
    case ITEM_NONE:       return "NONE";
    case ITEM_CLUB:       return "Club";
    case ITEM_DAGGER:     return "Dagger";
    case ITEM_SLING:      return "Sling";
    case ITEM_BROADSWORD: return "Broadsword";
    case ITEM_SHORTSWORD: return "Shortsword";
    case ITEM_GREATSWORD: return "Greatsword";
    case ITEM_SPEAR:      return "Spear";
    case ITEM_AXE:        return "Axe";
    case ITEM_HANDAXE:    return "Handaxe";
    case ITEM_BOW:        return "Bow";
    case ITEM_LONGBOW:    return "Longbow";
    case ITEM_SPARK:      return "Spark";
    case ITEM_EMBER:      return "Ember";
    case ITEM_GUST:       return "Gust";
    case ITEM_STAFF_HEAL: return "Staff of Healing";
    case ITEM_STAFF_MEND: return "Staff of Mending";
    case ITEM_STAFF_REVIVE:return "Staff of Revive";
    case ITEM_POTION_LOW: return "Potion I";
    case ITEM_POTION_MID: return "Potion II";
    case ITEM_POTION_HIGH:return "Potion III";
    case ITEM_SHIELD:     return "Shield";
    case ITEM_HELMET:     return "Helmet";
    case ITEM_WIND_RING:  return "Ring of Wind";
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
        case ITEM_CLUB:           return new Item(type, new WeaponComponent(d4));
        case ITEM_DAGGER:         return new Item(type, new WeaponComponent(d4, 1, STAT_DEXTERITY, STAT_DEXTERITY, 1, 4));
        case ITEM_SLING:          return new Item(type, new WeaponComponent(d4, 1, STAT_DEXTERITY, STAT_DEXTERITY, 2, 3));

        case ITEM_BROADSWORD:     return new Item(type, new WeaponComponent(d6));
        case ITEM_SHORTSWORD:     return new Item(type, new WeaponComponent(d6, 1, STAT_DEXTERITY, STAT_DEXTERITY));
        case ITEM_GREATSWORD:     return new Item(type, new WeaponComponent(d6, 2));

        case ITEM_SPEAR:          return new Item(type, new WeaponComponent(d6, 1, STAT_STRENGTH, STAT_STRENGTH, 1, 2));

        case ITEM_AXE:            return new Item(type, new WeaponComponent(d8));
        case ITEM_HANDAXE:        return new Item(type, new WeaponComponent(d6, 1, STAT_STRENGTH, STAT_STRENGTH, 2, 4));

        case ITEM_BOW:            return new Item(type, new WeaponComponent(d6, 1, STAT_DEXTERITY, STAT_DEXTERITY, 2, 6));
        case ITEM_LONGBOW:        return new Item(type, new WeaponComponent(d12, 1, STAT_DEXTERITY, STAT_DEXTERITY, 3, 10));

        case ITEM_SPARK:          return new Item(type, new WeaponComponent(d6, 1, STAT_INTUITION, STAT_INTUITION, 2, 4));
        case ITEM_EMBER:          return new Item(type, new WeaponComponent(d6, 1, STAT_INTUITION, STAT_INTUITION, 2, 4));
        case ITEM_GUST:           return new Item(type, new WeaponComponent(d6, 1, STAT_INTUITION, STAT_INTUITION, 2, 4));

        case ITEM_STAFF_HEAL:     return new Item(type, new WeaponComponent(d6, 1, STAT_FAITH, STAT_FAITH, 1, 1));
        case ITEM_STAFF_MEND:     return new Item(type, new WeaponComponent(d6, 1, STAT_FAITH, STAT_FAITH, 1, 1));
        case ITEM_STAFF_REVIVE:   return new Item(type, new WeaponComponent(d6, 1, STAT_FAITH, STAT_FAITH, 1, 1));

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
