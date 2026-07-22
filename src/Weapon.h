#pragma once

class Weapon
{
public:
    Weapon();

    void StartAttack();
    void Update(float deltaTime);

    bool IsAttacking() const;

private:
    bool attacking;
    float attackTimer;
    float attackDuration;
};
