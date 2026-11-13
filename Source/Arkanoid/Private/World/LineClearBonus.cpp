#include "World/LineClearBonus.h"

#include "World/PlayingBoard.h"

ALineClearBonus::ALineClearBonus()
{
    Color = FLinearColor(0.97f, 0.54f, 0.30f);
    Symbol = TEXT("|");
}

void ALineClearBonus::ApplyTo(APaddle&) const
{
    if (APlayingBoard* Board = Cast<APlayingBoard>(GetOwner()))
    {
        Board->DestroyColumn(GetSourceColumn());
    }
}
