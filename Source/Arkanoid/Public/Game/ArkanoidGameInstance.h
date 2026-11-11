#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ArkanoidGameInstance.generated.h"

UCLASS()
class ARKANOID_API UArkanoidGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Score")
    int32 GetBestScore() const { return BestScore; }

    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Score")
    bool RecordScore(int32 Score);

private:
    int32 BestScore = 0;
};
