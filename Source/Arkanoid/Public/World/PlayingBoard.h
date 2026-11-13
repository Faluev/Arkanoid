#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayingBoard.generated.h"

class ABlock;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UENUM(BlueprintType)
enum class EArkanoidBoardLayout : uint8
{
    FullGrid,
    Diamond,
    SplitWings
};

UCLASS()
class ARKANOID_API APlayingBoard : public AActor
{
    GENERATED_BODY()

public:
    APlayingBoard();

    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintPure, Category = "Board")
    int32 GetRemainingBlocks() const;

    void DestroyRandomBlocks(int32 Count);
    void DestroyColumn(int32 Column);
    bool UsesDiplomaBonuses() const { return bEnableDiplomaBonuses; }

protected:
    virtual void BeginPlay() override;

private:
    bool HasCell(int32 Row, int32 Column) const;
    FTransform GetBlockTransform(int32 Row, int32 Column) const;

    UFUNCTION()
    void HandleBlockDestroyed(AActor* DestroyedActor);

    UPROPERTY(VisibleAnywhere, Category = "Board")
    TObjectPtr<UInstancedStaticMeshComponent> Preview;

    UPROPERTY(EditAnywhere, Category = "Board")
    TSubclassOf<ABlock> BlockClass;

    UPROPERTY(EditAnywhere, Category = "Board", meta = (ClampMin = "1", ClampMax = "12"))
    int32 Columns = 6;

    UPROPERTY(EditAnywhere, Category = "Board", meta = (ClampMin = "1", ClampMax = "12"))
    int32 Rows = 3;

    UPROPERTY(EditAnywhere, Category = "Board", meta = (ClampMin = "120"))
    float ColumnSpacing = 145.0f;

    UPROPERTY(EditAnywhere, Category = "Board", meta = (ClampMin = "85"))
    float RowSpacing = 110.0f;

    UPROPERTY(EditAnywhere, Category = "Board")
    EArkanoidBoardLayout Layout = EArkanoidBoardLayout::FullGrid;

    UPROPERTY(EditAnywhere, Category = "Board")
    FVector BlockScale = FVector(0.84, 1.16, 0.50);

    UPROPERTY(EditAnywhere, Category = "Board")
    FLinearColor BoardColor = FLinearColor(0.26f, 0.44f, 0.57f);

    UPROPERTY(EditAnywhere, Category = "Board")
    TObjectPtr<UStaticMesh> BlockMesh;

    UPROPERTY(EditAnywhere, Category = "Board")
    TObjectPtr<UMaterialInterface> BlockMaterial;

    UPROPERTY(EditAnywhere, Category = "Board")
    bool bEnableDiplomaBonuses = false;

    UPROPERTY(Transient)
    TArray<TObjectPtr<ABlock>> Blocks;
};
