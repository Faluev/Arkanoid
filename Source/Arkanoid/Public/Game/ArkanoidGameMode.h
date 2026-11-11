#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArkanoidGameMode.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS()
class ARKANOID_API AArkanoidGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AArkanoidGameMode();

    virtual void StartPlay() override;

    void PlayMenuClick() const;

protected:
    bool bArcadePresentation = false;

private:
    UPROPERTY(Transient)
    TObjectPtr<USoundBase> MenuClick;
};

UCLASS()
class ARKANOID_API AArkanoidArcadeMode : public AArkanoidGameMode
{
    GENERATED_BODY()

public:
    AArkanoidArcadeMode();

    virtual void StartPlay() override;

    void OnBlockDestroyed(const FVector& Location, bool bBoardCleared);
    void OnBonusCollected(const FVector& Location) const;
    void OnLivesDepleted();

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Score")
    int32 GetScore() const { return Score; }

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Score")
    int32 GetBestScore() const;

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Round")
    bool HasFinished() const { return bFinished; }

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Round")
    bool HasWon() const { return bWon; }

private:
    void FinishRound(bool bVictory);

    UPROPERTY(Transient)
    TObjectPtr<USoundBase> BlockSound;

    UPROPERTY(Transient)
    TObjectPtr<USoundBase> PickupSound;

    UPROPERTY(Transient)
    TObjectPtr<UParticleSystem> BlockEffect;

    int32 Score = 0;
    bool bFinished = false;
    bool bWon = false;
};

UCLASS()
class ARKANOID_API AArkanoidMenuMode : public AArkanoidGameMode
{
    GENERATED_BODY()

public:
    AArkanoidMenuMode();

    virtual void StartPlay() override;
};
