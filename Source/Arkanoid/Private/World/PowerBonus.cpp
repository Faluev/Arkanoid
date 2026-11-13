#include "World/PowerBonus.h"

#include "World/Ball.h"
#include "World/Paddle.h"

APowerBonus::APowerBonus()
{
    Color = FLinearColor(0.62f, 0.31f, 1.00f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "Power", "Power ball");
    Symbol = TEXT("POW");
}

void APowerBonus::ApplyTo(APaddle& Paddle) const
{
    if (ABall* Ball = Paddle.GetCurrentBall())
    {
        Ball->ApplyPowerBonus(1, 9.0f);
    }
}
