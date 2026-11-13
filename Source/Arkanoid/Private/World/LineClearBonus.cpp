#include "World/LineClearBonus.h"

#include "World/PlayingBoard.h"

ALineClearBonus::ALineClearBonus()
{
    Color = FLinearColor(0.97f, 0.54f, 0.30f);
    DisplayName = NSLOCTEXT("ArkanoidBonus", "LineClear", "Line clear");
    Symbol = TEXT("|");
}

void ALineClearBonus::ApplyTo(APaddle&) const
{
    if (APlayingBoard* Board = Cast<APlayingBoard>(GetOwner()))
    {
        Board->DestroyColumn(GetSourceColumn());
    }
}
