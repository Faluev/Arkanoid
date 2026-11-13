#include "World/StickyBallBonus.h"

#include "World/Paddle.h"

AStickyBallBonus::AStickyBallBonus()
{
    Color = FLinearColor(0.48f, 0.85f, 1.0f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "Sticky", "Sticky ball");
    Symbol = TEXT("S");
}

void AStickyBallBonus::ApplyTo(APaddle& Paddle) const
{
    Paddle.ActivateStickyBall();
}
