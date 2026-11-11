#include "Game/ArkanoidGameMode.h"

#include "Game/ArkanoidHUD.h"

AArkanoidGameMode::AArkanoidGameMode()
{
    // The arena contains a player-owned paddle at its authored starting position.
    DefaultPawnClass = nullptr;
    HUDClass = AArkanoidHUD::StaticClass();
}
