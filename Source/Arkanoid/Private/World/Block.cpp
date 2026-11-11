#include "World/Block.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Ball.h"
#include "World/BonusParent.h"
#include "World/LifeComponent.h"

ABlock::ABlock()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetRelativeScale3D(FVector(0.84, 1.16, 0.50));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionObjectType(ECC_WorldStatic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        Mesh->SetStaticMesh(Cube.Object);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Tint(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Tint.Succeeded())
    {
        Mesh->SetMaterial(0, Tint.Object);
    }

    LifeComponent = CreateDefaultSubobject<ULifeComponent>(TEXT("Life"));
}

void ABlock::Init(int32 LifeAmount, TSubclassOf<ABonusParent> NewBonusClass, const FLinearColor& NewColor)
{
    LifeComponent->SetLife(LifeAmount);
    BonusClass = NewBonusClass;
    Color = NewColor;
}

void ABlock::BeginPlay()
{
    Super::BeginPlay();
    InitialLife = LifeComponent->GetLife();
    Material = Mesh->CreateDynamicMaterialInstance(0);
    UpdateAppearance();
}

int32 ABlock::GetLife() const
{
    return LifeComponent->GetLife();
}

void ABlock::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
    bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    const ABall* Ball = Cast<ABall>(Other);
    if (!Ball || IsActorBeingDestroyed())
    {
        return;
    }

    LifeComponent->ApplyDamage(Ball->GetPower());
    UE_LOG(LogTemp, Log, TEXT("Arkanoid Block %s hit, life %d"), *GetName(), GetLife());
    if (GetLife() > 0)
    {
        UpdateAppearance();
        return;
    }

    if (UWorld* World = GetWorld(); World && BonusClass)
    {
        // The board owns the bonus so it can affect this board's blocks.
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.Owner = GetOwner();
        World->SpawnActor<ABonusParent>(BonusClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
    }
    Destroy();
}

void ABlock::UpdateAppearance()
{
    if (Material)
    {
        const float HealthFraction = static_cast<float>(GetLife()) / InitialLife;
        const float Brightness = FMath::Lerp(0.07f, 0.20f, HealthFraction);
        Material->SetVectorParameterValue(TEXT("Color"), Color * Brightness);
    }
}
