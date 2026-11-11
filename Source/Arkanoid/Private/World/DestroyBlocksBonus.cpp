#include "World/DestroyBlocksBonus.h"

#include "World/PlayingBoard.h"

ADestroyBlocksBonus::ADestroyBlocksBonus()
{
    Color = FLinearColor(1.00f, 0.31f, 0.16f);
    Symbol = TEXT("-3");
}

void ADestroyBlocksBonus::ApplyTo(APaddle&) const
{
    if (APlayingBoard* Board = Cast<APlayingBoard>(GetOwner()))
    {
        Board->DestroyRandomBlocks(3);
    }
}
