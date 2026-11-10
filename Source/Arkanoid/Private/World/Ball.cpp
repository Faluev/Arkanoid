#include "World/Ball.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABall::ABall()
{
    PrimaryActorTick.bCanEverTick = true;

    constexpr float BallRadius = 22.0f;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(BallRadius);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetGenerateOverlapEvents(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (Sphere.Succeeded())
    {
        Mesh->SetStaticMesh(Sphere.Object);
        Mesh->SetRelativeScale3D(FVector(BallRadius / 50.0f));
    }
}

void ABall::BeginPlay()
{
    Super::BeginPlay();
    Direction = GetActorForwardVector().GetSafeNormal2D();
}

void ABall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    double Distance = Speed * DeltaTime;
    // A second sweep uses the part of this frame's travel left after a bounce.
    for (int32 Sweep = 0; Sweep < 2 && Distance > 0.0; ++Sweep)
    {
        FHitResult Hit;
        AddActorWorldOffset(Direction * Distance, true, &Hit);
        if (IsActorBeingDestroyed() || !Hit.IsValidBlockingHit())
        {
            return;
        }

        // Reverse the component of travel along the surface normal.
        Direction = (Direction - 2.0 * FVector::DotProduct(Direction, Hit.Normal) * Hit.Normal).GetSafeNormal2D();
        Distance *= 1.0 - Hit.Time;
        UE_LOG(LogTemp, Log, TEXT("Arkanoid Ball %s reflected"), *GetName());
    }
}
