#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayingBoard.generated.h"

class ABlock;
class UInstancedStaticMeshComponent;

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

protected:
    virtual void BeginPlay() override;

private:
    FTransform GetBlockTransform(int32 Row, int32 Column) const;

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

    UPROPERTY(Transient)
    TArray<TObjectPtr<ABlock>> Blocks;
};
