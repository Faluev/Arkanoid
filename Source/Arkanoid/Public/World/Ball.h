#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ball.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ARKANOID_API ABall : public AActor
{
    GENERATED_BODY()

public:
    ABall();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Ball")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, Category = "Ball")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(EditAnywhere, Category = "Ball", meta = (ClampMin = "1.0"))
    float Speed = 550.0f;

    FVector Direction = FVector::ForwardVector;
};
