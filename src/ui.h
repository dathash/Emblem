// Author: Alex Hartford
// Program: Emblem
// File: UI
// Date: December 2022

#ifndef UI_H
#define UI_H

// ============================ New UI =========================================
struct UI_State
{
    bool tile_info = false;
    bool unit_info = false;
    bool combat_preview = false;
    bool game_menu = false;
    bool unit_menu = false;
    bool combat_screen = false;

    void Update()
    {
        if(GlobalInterfaceState == NEUTRAL_OVER_ENEMY || 
           GlobalInterfaceState == NEUTRAL_OVER_UNIT ||
           GlobalInterfaceState == NEUTRAL_OVER_DEACTIVATED_UNIT ||
           GlobalInterfaceState == SELECTED_OVER_ALLY ||
           GlobalInterfaceState == SELECTED_OVER_ENEMY ||
           GlobalInterfaceState == ATTACK_TARGETING_OVER_TARGET ||
           GlobalInterfaceState == HEALING_TARGETING_OVER_TARGET ||
           GlobalInterfaceState == GAME_MENU_ROOT ||
           GlobalInterfaceState == UNIT_MENU_ROOT)
        {
            tile_info = true;
        }
        else
        {
            tile_info = false;
        }
    }
};

void DisplayTileInfo()
{
    ImGui::Begin("Tile Info");//ImGuiWindowFlags_AlwaysAutoResize);
    {
        char width[6] = "", height[6] = "";
        ImGui::InputText("World Width", width, 6);
        ImGui::InputText("World Height", height, 6);
        ImGui::Button("Cancel");
        ImGui::SameLine();
        ImGui::Button("Create");
    }
    ImGui::End();
}

void RenderUI(UI_State *ui)
{
    // Internal variables
    //

    if(ui->tile_info)
    {
        DisplayTileInfo();
    }
    //ImGui::ShowDemoWindow();
}

// ================================= Old UI ====================================
struct Menu
{
    u8 rows;
    u8 current;

    vector<Texture> optionTextTextures;

    Menu(u8 rows_in, u8 current_in, vector<string> options_in)
    : rows(rows_in),
      current(current_in)
    {
        for(string s : options_in)
        {
            optionTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};

struct UnitInfo
{
    u8 rows;
    vector<Texture> infoTextTextures;

    UnitInfo(u8 rows_in, vector<string> info_in)
    : rows(rows_in)
    {
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }

    void UpdateTextTextures(vector<string> info_in)
    {
        infoTextTextures.clear();
        int newRows = 0;
        for(string s : info_in)
        {
            infoTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
    }
};

struct CombatInfo
{
    u8 rows;

    vector<Texture> sourceTextTextures;
    vector<Texture> targetTextTextures;

    int unitHp = 5;
    int unitMaxHp = 10;
    int enemyHp = 5;
    int enemyMaxHp = 10;

    int unitDamage = 0;
    int enemyDamage = 0;

    bool attackerAttacking = false;
    bool victimAttacking = false;

    // TODO: Remove!!!
    // Determines what damage a hit will do.
    int CalculateDamage(int attack, int defense)
    {
        return max(attack - defense, 0);
    }

    CombatInfo(u8 rows_in, vector<string> sourceInfo_in, vector<string> targetInfo_in)
    : rows(rows_in)
    {
        for(string s : sourceInfo_in)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }

        for(string s : targetInfo_in)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        } 
    }

    void UpdatePreview(Unit &unit, Unit &enemy)
    {
        unitHp = unit.hp;
        unitMaxHp = unit.maxHp;
        enemyHp = enemy.hp;
        enemyMaxHp = enemy.maxHp;

        int dist = ManhattanDistance(point(unit.col, unit.row), point(enemy.col, enemy.row));
        attackerAttacking = dist >= unit.minRange && dist <= unit.maxRange; 
        victimAttacking = dist >= enemy.minRange && dist <= enemy.maxRange; 
        printf("%d %d\n", attackerAttacking, victimAttacking);

        if(attackerAttacking)
        {
            unitDamage = CalculateDamage(unit.attack, enemy.defense);
        }
        else
        {
            unitDamage = 0;
        }
        if(victimAttacking)
        {
            enemyDamage = CalculateDamage(enemy.attack, unit.defense);
        }
        else
        {
            enemyDamage = 0;
        }

        vector<string> unitInfo =
        {
            unit.name,
            "N/A",
            "Hit: " + to_string(unit.accuracy),
            "Dmg: " + to_string(unitDamage),
        };

        vector<string> enemyInfo =
        {
            enemy.name,
            "N/A",
            "Hit: " + to_string(enemy.accuracy),
            "Dmg: " + to_string(enemyDamage),
        };

        sourceTextTextures.clear();
        targetTextTextures.clear();
        // Update textures with given strings
        int newRows = 0;
        for(string s : unitInfo)
        {
            sourceTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
            ++newRows;
        }
        this->rows = newRows;
        for(string s : enemyInfo)
        {
            targetTextTextures.push_back(LoadTextureText(s.c_str(), {250, 250, 250, 255}));
        }
    }
};

#endif
