#include "World/ShieldBonus.h"

#include "World/Paddle.h"

AShieldBonus::AShieldBonus()
{
    Color = FLinearColor(0.96f, 0.26f, 0.61f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "Shield", "Shield");
    Symbol = TEXT("SH");
}

void AShieldBonus::ApplyTo(APaddle& Paddle) const
{
    Paddle.ActivateShield();
}
