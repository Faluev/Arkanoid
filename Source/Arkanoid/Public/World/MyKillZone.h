#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyKillZone.generated.h"

class UBoxComponent;

UCLASS()
class ARKANOID_API AMyKillZone : public AActor
{
    GENERATED_BODY()

public:
    AMyKillZone();

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Kill Zone")
    TObjectPtr<UBoxComponent> Collision;
};
