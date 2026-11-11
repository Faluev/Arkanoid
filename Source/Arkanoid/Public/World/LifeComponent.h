#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LifeComponent.generated.h"

UCLASS(ClassGroup = (Arkanoid), meta = (BlueprintSpawnableComponent))
class ARKANOID_API ULifeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULifeComponent();

    void SetLife(int32 Amount);
    void ApplyDamage(int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Life")
    int32 GetLife() const { return Life; }

private:
    UPROPERTY(EditAnywhere, Category = "Life", meta = (ClampMin = "1"))
    int32 Life = 1;
};
