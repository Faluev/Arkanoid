#include "World/MyKillZone.h"

#include "World/Ball.h"
#include "Components/BoxComponent.h"

AMyKillZone::AMyKillZone()
{
    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitBoxExtent(FVector(95.0, 125.0, 100.0));
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldStatic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);
}

void AMyKillZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    // Other dynamic actors can overlap the box, but only balls are removed.
    if (ABall* Ball = Cast<ABall>(OtherActor))
    {
        const FString BallName = Ball->GetName();
        if (Ball->Destroy())
        {
            UE_LOG(LogTemp, Log, TEXT("Arkanoid Ball %s destroyed in kill zone"), *BallName);
        }
    }
}
