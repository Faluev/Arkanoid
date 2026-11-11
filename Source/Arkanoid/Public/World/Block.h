#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

class ABonusParent;
class ULifeComponent;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class ARKANOID_API ABlock : public AActor
{
    GENERATED_BODY()

public:
    ABlock();

    void Init(int32 LifeAmount, TSubclassOf<ABonusParent> NewBonusClass, const FLinearColor& NewColor);

    UFUNCTION(BlueprintPure, Category = "Block")
    int32 GetLife() const;

    UFUNCTION(BlueprintPure, Category = "Block")
    TSubclassOf<ABonusParent> GetBonusClass() const { return BonusClass; }

protected:
    virtual void BeginPlay() override;
    virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
        bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

private:
    void UpdateAppearance();

    UPROPERTY(VisibleAnywhere, Category = "Block")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, Category = "Block")
    TObjectPtr<ULifeComponent> LifeComponent;

    UPROPERTY(EditAnywhere, Category = "Block")
    TSubclassOf<ABonusParent> BonusClass;

    UPROPERTY(EditAnywhere, Category = "Block")
    FLinearColor Color = FLinearColor(0.26f, 0.44f, 0.57f);

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> Material;

    int32 InitialLife = 1;
};
