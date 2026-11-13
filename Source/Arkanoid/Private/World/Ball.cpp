#include "World/Ball.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Paddle.h"

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
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Tint(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    TintMaterial = Tint.Object;
}

void ABall::BeginPlay()
{
    Super::BeginPlay();
    Direction = GetActorForwardVector().GetSafeNormal2D();
    if (Cast<APaddle>(GetOwner()))
    {
        Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        if (TintMaterial)
        {
            ActiveMaterial = Mesh->CreateDynamicMaterialInstance(0, TintMaterial);
            UpdatePowerTint();
        }
    }
}

void ABall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bLaunched)
    {
        return;
    }

    if (bBypassingPaddle)
    {
        AActor* BallOwner = GetOwner();
        if (!BallOwner || GetActorLocation().X > BallOwner->GetActorLocation().X + 60.0)
        {
            Collision->IgnoreActorWhenMoving(BallOwner, false);
            bBypassingPaddle = false;
        }
    }

    double Distance = GetSpeed() * DeltaTime;
    // A second sweep uses the part of this frame's travel left after a bounce.
    for (int32 Sweep = 0; Sweep < 2 && Distance > 0.0; ++Sweep)
    {
        bShieldBounceThisMove = false;
        FHitResult Hit;
        AddActorWorldOffset(Direction * Distance, true, &Hit);
        if (IsActorBeingDestroyed() || bShieldBounceThisMove || !Hit.IsValidBlockingHit())
        {
            return;
        }

        if (APaddle* Paddle = Cast<APaddle>(Hit.GetActor()); Paddle && Hit.Normal.X > 0.5)
        {
            if (Direction.X < 0.0 && Paddle->TryCatchBall(this))
            {
                return;
            }
            const double Offset = (GetActorLocation().Y - Paddle->GetActorLocation().Y) / Paddle->GetHalfWidth();
            Direction = FVector(1.0, FMath::Clamp(Offset, -1.0, 1.0), 0.0).GetSafeNormal2D();
        }
        else
        {
            // Reverse the component of travel along the surface normal.
            Direction = (Direction - 2.0 * FVector::DotProduct(Direction, Hit.Normal) * Hit.Normal).GetSafeNormal2D();
        }
        Distance *= 1.0 - Hit.Time;
        UE_LOG(LogTemp, Log, TEXT("Arkanoid Ball %s reflected"), *GetName());
    }
}

void ABall::HoldForLaunch()
{
    bLaunched = false;
    SetActorEnableCollision(false);
}

void ABall::Launch(const FVector& LaunchDirection)
{
    Direction = LaunchDirection.GetSafeNormal2D();
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    bLaunched = true;
    SetActorEnableCollision(true);
}

void ABall::BounceFromShield()
{
    // The saved ball crosses the back of the paddle on its way up the board.
    Collision->IgnoreActorWhenMoving(GetOwner(), true);
    bBypassingPaddle = true;
    bShieldBounceThisMove = true;
    Direction.X = FMath::Max(FMath::Abs(Direction.X), 0.45);
    Direction = Direction.GetSafeNormal2D();
}

void ABall::ApplySpeedBonus(float Multiplier, float Duration)
{
    if (Multiplier > 0.0f && Duration > 0.0f)
    {
        SpeedMultiplier = Multiplier;
        GetWorldTimerManager().SetTimer(SpeedBonusTimer, this, &ABall::ResetSpeedBonus, Duration, false);
    }
}

void ABall::ApplyPowerBonus(int32 ExtraPower, float Duration)
{
    if (ExtraPower > 0 && Duration > 0.0f)
    {
        Power = 1 + ExtraPower;
        UpdatePowerTint();
        GetWorldTimerManager().SetTimer(PowerBonusTimer, this, &ABall::ResetPowerBonus, Duration, false);
    }
}

void ABall::ResetSpeedBonus()
{
    SpeedMultiplier = 1.0f;
}

void ABall::ResetPowerBonus()
{
    Power = 1;
    UpdatePowerTint();
}

void ABall::UpdatePowerTint()
{
    if (ActiveMaterial)
    {
        const FLinearColor Tint = Power > 1 ? FLinearColor(0.124f, 0.062f, 0.20f) : FLinearColor::White;
        ActiveMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
    }
}
