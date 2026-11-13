#include "World/SizeBonus.h"

#include "World/Paddle.h"

ASizeBonus::ASizeBonus()
{
    Color = FLinearColor(0.20f, 0.85f, 0.52f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "Size", "Wider paddle");
    Symbol = TEXT("SIZE");
}

void ASizeBonus::ApplyTo(APaddle& Paddle) const
{
    Paddle.ApplySizeBonus(1.6f, 9.0f);
}
