#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ArkanoidHUD.generated.h"

class APlayingBoard;
class AArkanoidArcadeController;
class AArkanoidArcadeMode;
class APaddle;
class UFont;
class UTexture2D;

UCLASS()
class ARKANOID_API AArkanoidHUD : public AHUD
{
    GENERATED_BODY()

public:
    AArkanoidHUD();
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;

private:
    void DrawOrbital(const AArkanoidArcadeController& Controller, const APaddle* Paddle);
    void DrawOrbitalFeedback(const AArkanoidArcadeMode& Arcade);

    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> OrbitalMenuArt;

    UPROPERTY()
    TObjectPtr<UFont> OrbitalFont;

    bool bMenuArtLoaded = false;
    TWeakObjectPtr<APlayingBoard> PlayingBoard;
    int32 StartingBlockCount = 0;
};
