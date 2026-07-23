#include "Weapon.h"

Weapon::Weapon()
{
    attacking = false;
    attackTimer = 0.0f;
    attackDuration = 0.25f;
}

void Weapon::StartAttack()
{
    if (!attacking)
    {
        attacking = true;
        attackTimer = attackDuration;
    }
}

void Weapon::Update(float deltaTime)
{
    if (attacking)
    {
        attackTimer -= deltaTime;

        if (attackTimer <= 0.0f)
        {
            attacking = false;
            attackTimer = 0.0f;
        }
    }
}

bool Weapon::IsAttacking() const
{
    return attacking;
}

float Weapon::GetAttackProgress() const
{
    if (!attacking)
    {
        return 0.0f;
    }

    return 1.0f - (attackTimer / attackDuration);
}