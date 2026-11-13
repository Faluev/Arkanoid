#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Paddle.generated.h"

class ABall;
class UBoxComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UStaticMeshComponent;
struct FInputActionValue;

UCLASS()
class ARKANOID_API APaddle : public APawn
{
    GENERATED_BODY()

public:
    APaddle();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Paddle")
    void LaunchBall();

    UFUNCTION(BlueprintCallable, Category = "Paddle")
    void ApplySizeBonus(float Multiplier, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Paddle")
    void AddLife(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Paddle")
    void ActivateShield();

    UFUNCTION(BlueprintPure, Category = "Paddle")
    bool HasShield() const { return bShieldActive; }

    UFUNCTION(BlueprintCallable, Category = "Paddle")
    void ActivateStickyBall();

    UFUNCTION(BlueprintPure, Category = "Paddle")
    bool HasStickyBall() const { return bStickyBallActive; }

    UFUNCTION(BlueprintPure, Category = "Paddle")
    int32 GetLives() const { return Lives; }

    UFUNCTION(BlueprintPure, Category = "Paddle")
    ABall* GetCurrentBall() const { return CurrentBall; }

    UFUNCTION(BlueprintPure, Category = "Paddle")
    float GetSizeMultiplier() const { return SizeMultiplier; }

    UFUNCTION(BlueprintPure, Category = "Paddle")
    bool IsOutOfLives() const { return Lives == 0; }

    float GetHalfWidth() const;
    bool TrySaveBall(ABall* Ball);
    bool TryCatchBall(ABall* Ball);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void Move(const FInputActionValue& Value);
    void StopMoving();
    void RestartLevel();
    void PrepareBall();
    void HoldBall(ABall& Ball);
    void ResetSizeBonus();
    void SetPaddleSize(float Multiplier);

    UFUNCTION()
    void HandleBallDestroyed(AActor* DestroyedActor);

    UPROPERTY(VisibleAnywhere, Category = "Paddle")
    TObjectPtr<UBoxComponent> Collision;

    UPROPERTY(VisibleAnywhere, Category = "Paddle")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, Category = "Paddle")
    TObjectPtr<UStaticMeshComponent> ShieldMesh;

    UPROPERTY(EditAnywhere, Category = "Paddle", meta = (ClampMin = "1.0"))
    float MoveSpeed = 690.0f;

    UPROPERTY(EditAnywhere, Category = "Paddle", meta = (ClampMin = "80.0"))
    float LaneHalfWidth = 520.0f;

    UPROPERTY(EditAnywhere, Category = "Paddle", meta = (ClampMin = "1"))
    int32 StartingLives = 3;

    UPROPERTY(EditAnywhere, Category = "Paddle")
    TSubclassOf<ABall> BallClass;

    UPROPERTY()
    TObjectPtr<ABall> CurrentBall;

    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> InputMapping;

    TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
    FTimerHandle SizeBonusTimer;
    FTimerHandle PrepareBallTimer;
    float MovementInput = 0.0f;
    float SizeMultiplier = 1.0f;
    int32 Lives = 0;
    bool bShieldActive = false;
    bool bStickyBallActive = false;
};
