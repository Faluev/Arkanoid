#pragma once

#include "World/BonusParent.h"
#include "LineClearBonus.generated.h"

UCLASS()
class ARKANOID_API ALineClearBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    ALineClearBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
