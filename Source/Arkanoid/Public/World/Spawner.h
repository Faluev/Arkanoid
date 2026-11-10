#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Spawner.generated.h"

class ABall;
class UArrowComponent;

UCLASS()
class ARKANOID_API ASpawner : public AActor
{
    GENERATED_BODY()

public:
    ASpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void SpawnBall();

    UPROPERTY(VisibleAnywhere, Category = "Spawner")
    TObjectPtr<UArrowComponent> Arrow;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    TSubclassOf<ABall> BallClass;

    UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.1"))
    float SpawnInterval = 3.2f;

    FTimerHandle SpawnTimer;
};
