#include "World/LifeBonus.h"

#include "World/Paddle.h"

ALifeBonus::ALifeBonus()
{
    Color = FLinearColor(0.14f, 0.64f, 1.00f);
    Symbol = TEXT("+1");
}

void ALifeBonus::ApplyTo(APaddle& Paddle) const
{
    Paddle.AddLife(1);
}
