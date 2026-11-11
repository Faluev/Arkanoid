#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Ball.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class ARKANOID_API ABall : public AActor
{
    GENERATED_BODY()

public:
    ABall();

    virtual void Tick(float DeltaTime) override;

    void HoldForLaunch();
    void Launch(const FVector& LaunchDirection);
    void BounceFromShield();

    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ApplySpeedBonus(float Multiplier, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ApplyPowerBonus(int32 ExtraPower, float Duration);

    UFUNCTION(BlueprintPure, Category = "Ball")
    float GetSpeed() const { return Speed * SpeedMultiplier; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    int32 GetPower() const { return Power; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    bool IsLaunched() const { return bLaunched; }

protected:
    virtual void BeginPlay() override;

private:
    void ResetSpeedBonus();
    void ResetPowerBonus();
    void UpdatePowerTint();

    UPROPERTY(VisibleAnywhere, Category = "Ball")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, Category = "Ball")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> TintMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> ActiveMaterial;

    UPROPERTY(EditAnywhere, Category = "Ball", meta = (ClampMin = "1.0"))
    float Speed = 550.0f;

    FVector Direction = FVector::ForwardVector;
    FTimerHandle SpeedBonusTimer;
    FTimerHandle PowerBonusTimer;
    float SpeedMultiplier = 1.0f;
    int32 Power = 1;
    bool bLaunched = true;
    bool bBypassingPaddle = false;
    bool bShieldBounceThisMove = false;
};
