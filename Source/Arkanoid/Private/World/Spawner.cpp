#include "World/Spawner.h"

#include "World/Ball.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

ASpawner::ASpawner()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
    Arrow->SetupAttachment(GetRootComponent());
    BallClass = ABall::StaticClass();
}

void ASpawner::BeginPlay()
{
    Super::BeginPlay();
    SpawnBall();
    GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawner::SpawnBall, SpawnInterval, true);
}

void ASpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(SpawnTimer);
    Super::EndPlay(EndPlayReason);
}

void ASpawner::SpawnBall()
{
    // The arrow sets both the launch point and the ball's initial direction.
    if (ABall* Ball = GetWorld()->SpawnActor<ABall>(BallClass, Arrow->GetComponentLocation(), Arrow->GetComponentRotation()))
    {
        UE_LOG(LogTemp, Log, TEXT("Arkanoid Ball %s spawned"), *Ball->GetName());
    }
}
