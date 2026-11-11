#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ArkanoidHUD.generated.h"

class APlayingBoard;

UCLASS()
class ARKANOID_API AArkanoidHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    TWeakObjectPtr<APlayingBoard> PlayingBoard;
};
