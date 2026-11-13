#include "World/StickyBallBonus.h"

#include "World/Paddle.h"

AStickyBallBonus::AStickyBallBonus()
{
    Color = FLinearColor(0.48f, 0.85f, 1.0f);
    Symbol = TEXT("S");
}

void AStickyBallBonus::ApplyTo(APaddle& Paddle) const
{
    Paddle.ActivateStickyBall();
}
