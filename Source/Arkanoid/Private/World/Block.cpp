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
#include "World/PlayingBoard.h"

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

void ABlock::Init(int32 LifeAmount, TSubclassOf<ABonusParent> NewBonusClass, const FLinearColor& NewColor,
    int32 Row, int32 Column)
{
    LifeComponent->SetLife(LifeAmount);
    BonusClass = NewBonusClass;
    Color = NewColor;
    BoardRow = Row;
    BoardColumn = Column;
}

void ABlock::SetBoardAppearance(UStaticMesh* BlockMesh, UMaterialInterface* BlockMaterial)
{
    if (BlockMesh)
    {
        Mesh->SetStaticMesh(BlockMesh);
    }
    if (BlockMaterial)
    {
        Mesh->SetMaterial(0, BlockMaterial);
    }
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

    const APlayingBoard* Board = Cast<APlayingBoard>(GetOwner());
    // The diploma's power bonus clears armored blocks in one shot while active.
    const int32 Damage = Board && Board->UsesDiplomaBonuses() && Ball->GetPower() > 1 ? GetLife() : Ball->GetPower();
    LifeComponent->ApplyDamage(Damage);
    UE_LOG(LogTemp, Log, TEXT("Arkanoid Block %s hit, life %d"), *GetName(), GetLife());
    if (GetLife() > 0)
    {
        UpdateAppearance();
        return;
    }

    if (UWorld* World = GetWorld(); World && BonusClass)
    {
        // The board owns the bonus so it can affect this board's blocks.
        const FTransform BonusTransform(GetActorLocation());
        if (ABonusParent* Bonus = World->SpawnActorDeferred<ABonusParent>(BonusClass, BonusTransform,
            GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
        {
            Bonus->SetSourceCell(BoardRow, BoardColumn);
            Bonus->FinishSpawning(BonusTransform);
        }
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
