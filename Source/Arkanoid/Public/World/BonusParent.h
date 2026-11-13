#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BonusParent.generated.h"

class APaddle;
class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Abstract)
class ARKANOID_API ABonusParent : public AActor
{
    GENERATED_BODY()

public:
    ABonusParent();

    virtual void Tick(float DeltaTime) override;

    FLinearColor GetColor() const { return Color; }

    UFUNCTION(BlueprintCallable, Category = "Bonus")
    void SetSourceCell(int32 Row, int32 Column);

    UFUNCTION(BlueprintPure, Category = "Bonus")
    int32 GetSourceRow() const { return SourceRow; }

    UFUNCTION(BlueprintPure, Category = "Bonus")
    int32 GetSourceColumn() const { return SourceColumn; }

protected:
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    virtual void ApplyTo(APaddle& Paddle) const PURE_VIRTUAL(ABonusParent::ApplyTo, );

    FLinearColor Color = FLinearColor::White;
    FString Symbol;

private:
    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, Category = "Bonus")
    TObjectPtr<UTextRenderComponent> Label;

    UPROPERTY(EditAnywhere, Category = "Bonus", meta = (ClampMin = "1"))
    float FallSpeed = 350.0f;

    int32 SourceRow = INDEX_NONE;
    int32 SourceColumn = INDEX_NONE;
    bool bCollected = false;
};
