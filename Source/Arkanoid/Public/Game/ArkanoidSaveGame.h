#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ArkanoidSaveGame.generated.h"

UCLASS()
class ARKANOID_API UArkanoidSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    int32 BestScore = 0;
};
