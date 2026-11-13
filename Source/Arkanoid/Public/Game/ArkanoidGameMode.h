#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArkanoidGameMode.generated.h"

class UParticleSystem;
class USoundBase;

enum class EOrbitalFeedbackKind : uint8
{
    Launch,
    PaddleBounce,
    StickyCatch,
    ShieldSave,
    BlockHit,
    BlockDestroyed,
    BonusPickedUp,
    LineClear
};

struct FOrbitalFeedbackEvent
{
    FVector Position;
    FVector Direction;
    FLinearColor Color;
    float StartSeconds;
    EOrbitalFeedbackKind Kind;
    float Extent = 0.0f;
};

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

    void OnBlockDestroyed(const FVector& Location, bool bBoardCleared, const FLinearColor& Color);
    void OnBonusCollected(const FVector& Location, const FText& Name, const FLinearColor& Color);
    void OnLivesDepleted();
    void AddOrbitalFeedback(EOrbitalFeedbackKind Kind, const FVector& Position,
        const FVector& Direction = FVector::ZeroVector, const FLinearColor& Color = FLinearColor::White,
        float Extent = 0.0f);
    const TArray<FOrbitalFeedbackEvent>& GetOrbitalFeedback() const { return OrbitalFeedback; }
    const FText& GetLastBonusName() const { return LastBonusName; }

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Score")
    int32 GetScore() const { return Score; }

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Score")
    int32 GetBestScore() const;

    float GetRunTime() const;

    float GetBonusFlash() const;

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
    float StartSeconds = 0.0f;
    float FinalRunTime = 0.0f;
    float LastBonusTime = -10.0f;
    TArray<FOrbitalFeedbackEvent> OrbitalFeedback;
    FText LastBonusName;
    bool bOrbitalPresentation = false;
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
