#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BonusParent.generated.h"

class APaddle;
class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Abstract)
class ARKANOID_API ABonusParent : public AActor
{
    GENERATED_BODY()

public:
    ABonusParent();

    virtual void Tick(float DeltaTime) override;

    FLinearColor GetColor() const { return Color; }

protected:
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    virtual void ApplyTo(APaddle& Paddle) const PURE_VIRTUAL(ABonusParent::ApplyTo, );

    FLinearColor Color = FLinearColor::White;
    FString Symbol;

private:
    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<UTextRenderComponent> Label;

    UPROPERTY(EditAnywhere, Category = "Bonus", meta = (ClampMin = "1"))
    float FallSpeed = 350.0f;

    bool bCollected = false;
};
