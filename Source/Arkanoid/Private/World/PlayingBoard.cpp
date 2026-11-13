#include "World/PlayingBoard.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/ArkanoidGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Block.h"
#include "World/DestroyBlocksBonus.h"
#include "World/LifeBonus.h"
#include "World/LineClearBonus.h"
#include "World/PowerBonus.h"
#include "World/ShieldBonus.h"
#include "World/SizeBonus.h"
#include "World/SpeedBonus.h"
#include "World/StickyBallBonus.h"

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
    UStaticMesh* PreviewMesh = BlockMesh;
    if (!PreviewMesh)
    {
        PreviewMesh = GetDefault<APlayingBoard>()->Preview->GetStaticMesh();
    }
    Preview->SetStaticMesh(PreviewMesh);
    Preview->SetMaterial(0, BlockMaterial);

    // Editor preview and runtime spawns use the same cells and transforms.
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            if (HasCell(Row, Column))
            {
                Preview->AddInstance(GetBlockTransform(Row, Column));
            }
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

    const TSubclassOf<ABonusParent> OriginalBonuses[] = {
        ASizeBonus::StaticClass(), ALifeBonus::StaticClass(), ASpeedBonus::StaticClass(),
        APowerBonus::StaticClass(), ADestroyBlocksBonus::StaticClass(), AShieldBonus::StaticClass()
    };
    const TSubclassOf<ABonusParent> DiplomaBonuses[] = {
        ASizeBonus::StaticClass(), ALifeBonus::StaticClass(), AStickyBallBonus::StaticClass(), ALineClearBonus::StaticClass(),
        ASpeedBonus::StaticClass(), APowerBonus::StaticClass(), ADestroyBlocksBonus::StaticClass(), AShieldBonus::StaticClass()
    };
    TSet<int32> ColumnsWithBonus;
    int32 NextReserveBonus = Columns;

    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            if (!HasCell(Row, Column))
            {
                continue;
            }

            const FTransform BlockTransform = GetBlockTransform(Row, Column) * GetActorTransform();
            ABlock* Block = World->SpawnActorDeferred<ABlock>(BlockClass, BlockTransform, this, nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn, ESpawnActorScaleMethod::OverrideRootScale);
            if (Block)
            {
                TSubclassOf<ABonusParent> BonusClass;
                if (bEnableDiplomaBonuses)
                {
                    // The frontmost cell of each column drops a bonus even in a shaped layout.
                    if (!ColumnsWithBonus.Contains(Column))
                    {
                        ColumnsWithBonus.Add(Column);
                        BonusClass = DiplomaBonuses[Column % UE_ARRAY_COUNT(DiplomaBonuses)];
                    }
                    else if (NextReserveBonus < UE_ARRAY_COUNT(DiplomaBonuses))
                    {
                        BonusClass = DiplomaBonuses[NextReserveBonus++];
                    }
                }
                else if (Row == 0)
                {
                    BonusClass = OriginalBonuses[Column % UE_ARRAY_COUNT(OriginalBonuses)];
                }

                const FLinearColor Color = BonusClass ? BonusClass->GetDefaultObject<ABonusParent>()->GetColor() : BoardColor;
                const int32 Life = bEnableDiplomaBonuses ? FMath::Min(Row + 1, 3) : Row + 1;
                Block->Init(Life, BonusClass, Color, Row, Column);
                Block->SetBoardAppearance(BlockMesh, BlockMaterial);
                Block->FinishSpawning(BlockTransform, false, nullptr, ESpawnActorScaleMethod::OverrideRootScale);
                Blocks.Add(Block);
                Block->OnDestroyed.AddDynamic(this, &APlayingBoard::HandleBlockDestroyed);
            }
        }
    }
}

void APlayingBoard::HandleBlockDestroyed(AActor* DestroyedActor)
{
    ABlock* Block = Cast<ABlock>(DestroyedActor);
    const int32 Index = Blocks.IndexOfByKey(Block);
    if (Index == INDEX_NONE)
    {
        return;
    }

    // Remove first so every block can contribute points only once.
    Blocks.RemoveAtSwap(Index, 1, EAllowShrinking::No);
    UWorld* World = GetWorld();
    if (World && !World->bIsTearingDown)
    {
        if (AArkanoidArcadeMode* Mode = World->GetAuthGameMode<AArkanoidArcadeMode>())
        {
            Mode->OnBlockDestroyed(Block->GetActorLocation(), Blocks.IsEmpty(), Block->GetBlockColor());
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

void APlayingBoard::DestroyColumn(int32 Column)
{
    if (Column < 0 || Column >= Columns)
    {
        return;
    }

    // Destruction removes cells from Blocks, so collect the affected actors first.
    TArray<ABlock*> ColumnBlocks;
    const ABlock* FirstBlock = nullptr;
    const ABlock* LastBlock = nullptr;
    for (ABlock* Block : Blocks)
    {
        if (IsValid(Block) && Block->GetBoardColumn() == Column)
        {
            ColumnBlocks.Add(Block);
            if (!FirstBlock || Block->GetBoardRow() < FirstBlock->GetBoardRow())
            {
                FirstBlock = Block;
            }
            if (!LastBlock || Block->GetBoardRow() > LastBlock->GetBoardRow())
            {
                LastBlock = Block;
            }
        }
    }
    if (FirstBlock && LastBlock)
    {
        if (AArkanoidArcadeMode* Mode = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
        {
            const FVector FirstPosition = FirstBlock->GetActorLocation();
            const FVector ColumnSpan = LastBlock->GetActorLocation() - FirstPosition;
            Mode->AddOrbitalFeedback(EOrbitalFeedbackKind::LineClear, FirstPosition,
                ColumnSpan.IsNearlyZero() ? GetActorForwardVector() : ColumnSpan.GetSafeNormal(),
                FLinearColor(0.97f, 0.54f, 0.30f), static_cast<float>(ColumnSpan.Length()));
        }
    }
    for (ABlock* Block : ColumnBlocks)
    {
        if (IsValid(Block))
        {
            Block->Destroy();
        }
    }
}

bool APlayingBoard::HasCell(int32 Row, int32 Column) const
{
    if (Layout == EArkanoidBoardLayout::Diamond)
    {
        const int32 Indent = FMath::Clamp((Rows - 1) / 2 - FMath::Min(Row, Rows - 1 - Row), 0, (Columns - 1) / 2);
        return Column >= Indent && Column < Columns - Indent;
    }
    if (Layout == EArkanoidBoardLayout::SplitWings)
    {
        const bool bCenterGap = Columns >= 5 && Column == Columns / 2 && Row < FMath::Min(2, Rows - 1);
        const bool bTrimmedCorner = Rows >= 4 && Row == Rows - 1 && (Column == 0 || Column == Columns - 1);
        return !bCenterGap && !bTrimmedCorner;
    }
    return true;
}

FTransform APlayingBoard::GetBlockTransform(int32 Row, int32 Column) const
{
    const double CenteredColumn = Column - (Columns - 1) * 0.5;
    const FVector Location(Row * RowSpacing, CenteredColumn * ColumnSpacing, 0.0);
    return FTransform(FRotator::ZeroRotator, Location, BlockScale);
}
