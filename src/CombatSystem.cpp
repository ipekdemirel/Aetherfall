#include "CombatSystem.h"

#include "Enemy.h"
#include "Player.h"

CombatSystem::CombatSystem()
    : attackWasActive(false),
    attackDamage(25.0f),
    swordHitRadius(0.75f)
{
}

void CombatSystem::Update(Player& player, Enemy& enemy)
{
    // Şimdilik boş bırakıyoruz.
    // Bir sonraki adımda tüm saldırı mantığını
    // main.cpp'den buraya taşıyacağız.
}