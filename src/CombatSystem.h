#pragma once

#include <raylib.h>

class Player;
class Enemy;

class CombatSystem
{
public:
    CombatSystem();

    void Update(Player& player, Enemy& enemy);

private:
    bool attackWasActive;

    float attackDamage;
    float swordHitRadius;
};