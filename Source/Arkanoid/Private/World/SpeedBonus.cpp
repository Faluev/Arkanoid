#include "World/SpeedBonus.h"

#include "World/Ball.h"
#include "World/Paddle.h"

ASpeedBonus::ASpeedBonus()
{
    Color = FLinearColor(1.00f, 0.68f, 0.12f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "Speed", "Slow ball");
    Symbol = TEXT("SPD");
}

void ASpeedBonus::ApplyTo(APaddle& Paddle) const
{
    if (ABall* Ball = Paddle.GetCurrentBall())
    {
        Ball->ApplySpeedBonus(0.65f, 9.0f);
    }
}
