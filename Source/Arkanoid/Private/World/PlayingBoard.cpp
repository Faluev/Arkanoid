#include "World/PlayingBoard.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Block.h"
#include "World/DestroyBlocksBonus.h"
#include "World/LifeBonus.h"
#include "World/PowerBonus.h"
#include "World/ShieldBonus.h"
#include "World/SizeBonus.h"
#include "World/SpeedBonus.h"

APlayingBoard::APlayingBoard()
{
    PrimaryActorTick.bCanEverTick = false;

    Preview = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BlockPreview"));
    SetRootComponent(Preview);
    Preview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Preview->SetHiddenInGame(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        Preview->SetStaticMesh(Cube.Object);
    }
    BlockClass = ABlock::StaticClass();
}

void APlayingBoard::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Preview->ClearInstances();

    // Show the grid in the editor without placing gameplay actors.
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            Preview->AddInstance(GetBlockTransform(Row, Column));
        }
    }
}

void APlayingBoard::BeginPlay()
{
    Super::BeginPlay();
    Preview->ClearInstances();

    UWorld* World = GetWorld();
    if (!World || !BlockClass)
    {
        return;
    }

    // Put every bonus in the front row so each one is reachable at the start.
    const TSubclassOf<ABonusParent> BonusClasses[] = {
        ASizeBonus::StaticClass(), ALifeBonus::StaticClass(), ASpeedBonus::StaticClass(),
        APowerBonus::StaticClass(), ADestroyBlocksBonus::StaticClass(), AShieldBonus::StaticClass()
    };

    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            const FTransform BlockTransform = GetBlockTransform(Row, Column) * GetActorTransform();
            ABlock* Block = World->SpawnActorDeferred<ABlock>(BlockClass, BlockTransform, this, nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn, ESpawnActorScaleMethod::OverrideRootScale);
            if (Block)
            {
                TSubclassOf<ABonusParent> BonusClass;
                FLinearColor Color(0.26f, 0.44f, 0.57f);
                if (Row == 0)
                {
                    BonusClass = BonusClasses[Column % UE_ARRAY_COUNT(BonusClasses)];
                    Color = BonusClass->GetDefaultObject<ABonusParent>()->GetColor();
                }
                Block->Init(Row + 1, BonusClass, Color);
                Block->FinishSpawning(BlockTransform, false, nullptr, ESpawnActorScaleMethod::OverrideRootScale);
                Blocks.Add(Block);
            }
        }
    }
}

int32 APlayingBoard::GetRemainingBlocks() const
{
    int32 Remaining = 0;
    for (const ABlock* Block : Blocks)
    {
        if (IsValid(Block))
        {
            ++Remaining;
        }
    }
    return Remaining;
}

void APlayingBoard::DestroyRandomBlocks(int32 Count)
{
    TArray<ABlock*> LiveBlocks;
    for (ABlock* Block : Blocks)
    {
        if (IsValid(Block))
        {
            LiveBlocks.Add(Block);
        }
    }

    // Remove each choice from the candidates so a block is selected only once.
    const int32 BlocksToDestroy = FMath::Clamp(Count, 0, LiveBlocks.Num());
    for (int32 Index = 0; Index < BlocksToDestroy; ++Index)
    {
        const int32 Pick = FMath::RandRange(0, LiveBlocks.Num() - 1);
        LiveBlocks[Pick]->Destroy();
        LiveBlocks.RemoveAtSwap(Pick, 1, EAllowShrinking::No);
    }
}

FTransform APlayingBoard::GetBlockTransform(int32 Row, int32 Column) const
{
    const double CenteredColumn = Column - (Columns - 1) * 0.5;
    const FVector Location(Row * RowSpacing, CenteredColumn * ColumnSpacing, 0.0);
    return FTransform(FRotator::ZeroRotator, Location, FVector(0.84, 1.16, 0.50));
}
